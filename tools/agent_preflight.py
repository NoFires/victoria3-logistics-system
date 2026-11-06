#!/usr/bin/env python3
"""Single-writer preflight and shared claim for this repository.

The claim lives in the git common directory, so every worktree sees it.
Run before starting a work unit and release it when handing off.

Exit codes: 0 = allowed, 1 = blocked, 2 = usage/environment error.
"""
from __future__ import annotations

import argparse
import datetime as dt
import getpass
import json
import os
import subprocess
import sys
from pathlib import Path

for stream in (sys.stdout, sys.stderr):
    try:
        stream.reconfigure(encoding="utf-8", errors="replace")
    except AttributeError:
        pass

CANONICAL_BRANCH = os.environ.get("IRONROAD_CANONICAL_BRANCH", "main")
CLAIM_NAME = "claude-claim.json"
DEFAULT_HOURS = 8.0
UTC = dt.timezone.utc


def git(*args: str) -> str:
    return subprocess.run(
        ["git", *args], check=True, capture_output=True, text=True,
        encoding="utf-8", errors="replace",
    ).stdout.strip()


def common_dir() -> Path:
    return Path(git("rev-parse", "--git-common-dir")).resolve()


def repo_root() -> Path:
    return Path(git("rev-parse", "--show-toplevel")).resolve()


def current_branch() -> str:
    return git("rev-parse", "--abbrev-ref", "HEAD")


def current_status() -> str:
    return git("status", "--porcelain=v1")


def upstream_counts() -> tuple[int, int] | None:
    try:
        out = git("rev-list", "--left-right", "--count", "@{u}...HEAD")
    except subprocess.CalledProcessError:
        return None
    behind, ahead = (int(x) for x in out.split())
    return behind, ahead


def worktrees() -> list[dict[str, str]]:
    result: list[dict[str, str]] = []
    current: dict[str, str] = {}
    for line in git("worktree", "list", "--porcelain").splitlines():
        if not line:
            if current:
                result.append(current)
                current = {}
            continue
        key, _, value = line.partition(" ")
        current[key] = value
    if current:
        result.append(current)
    return result


def now() -> dt.datetime:
    return dt.datetime.now(UTC)


def iso(value: dt.datetime) -> str:
    return value.astimezone(UTC).isoformat(timespec="seconds")


def parse_claim(path: Path) -> dict | None:
    if not path.is_file():
        return None
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        print(f"claim 文件损坏，拒绝继续：{path}: {exc}")
        raise SystemExit(2)
    try:
        data["_expires"] = dt.datetime.fromisoformat(data["expires"])
    except (KeyError, ValueError):
        data["_expires"] = now() - dt.timedelta(seconds=1)
    return data


def owner_name(args: argparse.Namespace) -> str:
    return args.owner or os.environ.get("CLAUDE_SESSION_ID") or os.environ.get("CLAUDE_JOB_ID") or getpass.getuser()


def print_claim(claim: dict | None) -> None:
    if not claim:
        print("claim: 无")
        return
    state = "有效" if claim["_expires"] > now() else "已过期"
    print(
        f"claim: {state} owner={claim.get('owner')} branch={claim.get('branch')} "
        f"round={claim.get('round')} task={claim.get('task')} expires={claim.get('expires')}"
    )


def check(args: argparse.Namespace) -> tuple[dict | None, list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []
    branch = current_branch()
    status = current_status()
    claim_path = common_dir() / CLAIM_NAME
    claim = parse_claim(claim_path)

    if branch != CANONICAL_BRANCH and not args.allow_branch:
        errors.append(f"当前分支是 {branch}，正式线是 {CANONICAL_BRANCH}；实验分支请显式 --allow-branch")
    if status and not args.allow_dirty:
        errors.append("工作区有未提交改动；先提交，或确认改动属于本人后加 --allow-dirty")

    counts = upstream_counts()
    if counts is None:
        warnings.append("当前分支没有上游；无法检查落后/领先")
    else:
        behind, ahead = counts
        if behind:
            errors.append(f"落后上游 {behind} 个提交；先同步再开工")
        if ahead:
            warnings.append(f"领先上游 {ahead} 个提交；收工前推送")

    if claim and claim["_expires"] > now() and claim.get("owner") != owner_name(args):
        errors.append(
            f"已有其他人的有效 claim：owner={claim.get('owner')} branch={claim.get('branch')} "
            f"task={claim.get('task')} expires={claim.get('expires')}"
        )
    if claim and claim["_expires"] <= now():
        warnings.append("存在已过期 claim，可安全覆盖")

    return claim, errors, warnings


def describe(args: argparse.Namespace, claim: dict | None, warnings: list[str]) -> None:
    print(f"repo:   {repo_root()}")
    print(f"branch: {current_branch()}")
    print(f"round:  {args.round or (claim or {}).get('round', '-')}")
    print_claim(claim)
    for item in warnings:
        print(f"warning: {item}")
    others = [w for w in worktrees() if Path(w.get("worktree", "")).resolve() != repo_root()]
    if others:
        print("其他 worktree：")
        for item in others:
            print(
                f"  {item.get('worktree')}  {item.get('branch', '(detached)')}"
                f"{'  [locked]' if 'locked' in item else ''}"
            )


def write_claim(args: argparse.Namespace, claim: dict | None) -> None:
    path = common_dir() / CLAIM_NAME
    path.parent.mkdir(parents=True, exist_ok=True)
    started = now()
    data = {
        "owner": owner_name(args),
        "branch": current_branch(),
        "worktree": str(repo_root()),
        "base": git("rev-parse", "HEAD"),
        "task": args.claim,
        "round": args.round or (claim or {}).get("round", ""),
        "started": iso(started),
        "expires": iso(started + dt.timedelta(hours=args.hours)),
    }
    tmp = path.with_suffix(f".tmp{os.getpid()}")
    tmp.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(tmp, path)
    print(f"claim 已写入：{path}")
    print_claim({**data, "_expires": dt.datetime.fromisoformat(data["expires"])})


def release(args: argparse.Namespace) -> int:
    path = common_dir() / CLAIM_NAME
    claim = parse_claim(path)
    if not claim:
        print("没有 claim 可释放")
        return 0
    if claim.get("owner") != owner_name(args) and not args.force:
        print(f"拒绝释放他人的 claim：owner={claim.get('owner')}；如确需释放请加 --force")
        return 1
    path.unlink(missing_ok=True)
    print(f"claim 已释放：owner={claim.get('owner')} branch={claim.get('branch')}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--claim", metavar="TASK", help="登记本次工作并写入共享 claim")
    parser.add_argument("--release", action="store_true", help="释放本人 claim")
    parser.add_argument("--show", action="store_true", help="只显示当前 claim 与 worktree")
    parser.add_argument("--owner", help="claim 所有者（默认 CLAUDE_SESSION_ID/CLAUDE_JOB_ID/系统用户）")
    parser.add_argument("--round", help="预留的轮次号，例如 R221")
    parser.add_argument("--hours", type=float, default=DEFAULT_HOURS, help=f"claim 有效期，默认 {DEFAULT_HOURS:g} 小时")
    parser.add_argument("--allow-branch", action="store_true", help="允许非正式分支（实验分支）")
    parser.add_argument("--allow-dirty", action="store_true", help="允许工作区已有未提交改动")
    parser.add_argument("--force", action="store_true", help="释放 claim 时忽略所有者")
    args = parser.parse_args()

    if args.claim and args.release:
        parser.error("--claim 与 --release 不能同时使用")

    try:
        if args.release:
            return release(args)

        claim, errors, warnings = check(args)
        describe(args, claim, warnings)
        for item in errors:
            print(f"BLOCKED: {item}")
        if args.show:
            return 0 if not errors else 1
        if errors:
            return 1
        if args.claim:
            write_claim(args, claim)
        else:
            print("PRECHECK OK")
        return 0
    except subprocess.CalledProcessError as exc:
        print(f"git 调用失败：{exc}")
        return 2


if __name__ == "__main__":
    sys.exit(main())

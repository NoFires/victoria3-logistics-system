#pragma once
#include "native_sea_geography.h"
#include <tuple>
#include <type_traits>

namespace v3 {
// Exact owned-content comparison for snapshot handoff. No pointer-address or
// hash-only equality: a canal/choice/geometry change must reach the worker even
// if the legacy economy orders are identical. Only native semantic order
// (alternatives, controls, adjacency and passage arrays) affects equality.
inline bool NativeTradeEquivalent(const NativeWorldTradeSnapshot& a,const NativeWorldTradeSnapshot& b) {
  auto rows=[](const auto& left,const auto& right,const auto& equal){
    return left.size()==right.size() && std::equal(left.begin(),left.end(),right.begin(),equal);
  };
  auto byKey=[&](const auto& left,const auto& right,const auto& key,const auto& equal){
    if(left.size()!=right.size())return false;
    // Normal captures retain enumeration order: avoid allocating sorted views.
    if(rows(left,right,[&](const auto& x,const auto& y){return key(x)==key(y);}))return rows(left,right,equal);
    using Row=typename std::decay_t<decltype(left)>::value_type;
    std::vector<const Row*> first,second;first.reserve(left.size());second.reserve(right.size());
    for(const auto& row:left)first.push_back(&row);
    for(const auto& row:right)second.push_back(&row);
    const auto less=[&](auto x,auto y){return key(*x)<key(*y);};
    std::sort(first.begin(),first.end(),less);std::sort(second.begin(),second.end(),less);
    return rows(first,second,[&](auto x,auto y){return equal(*x,*y);});
  };
  auto amounts=[&](const auto& left,const auto& right){
    return byKey(left,right,[](const auto& x){return std::pair{x.source,x.destination};},[&](const auto& x,const auto& y){
      return x.source==y.source && x.destination==y.destination &&
          byKey(x.goods,y.goods,[](const auto& u){return u.good;},[](const auto& u,const auto& v){return u.good==v.good && u.valueFixed==v.valueFixed;});
    });
  };
  const auto geometry=[&](const NativeSplineNetwork& x,const NativeSplineNetwork& y){
    return x.width==y.width && x.height==y.height && x.strips==y.strips && x.connections==y.connections &&
        rows(x.points,y.points,[](const auto& u,const auto& v){
          return u.first==v.first && u.second.x==v.second.x && u.second.z==v.second.z;
        });
  };
  const auto sea=[&](const NativeSeaGeography& x,const NativeSeaGeography& y){
    const auto region=[](const auto& u,const auto& v){return u.definition==v.definition && u.geo==v.geo;};
    return x.stateHubs==y.stateHubs &&
        byKey(x.definitions,y.definitions,[](const auto& u){return u.id;},[](const auto& u,const auto& v){return u.id==v.id && u.seaProvinces==v.seaProvinces;}) &&
        rows(x.states,y.states,[&](const auto& u,const auto& v){return u.first==v.first && region(u.second,v.second);}) &&
        rows(x.provinces,y.provinces,[&](const auto& u,const auto& v){
          return u.first==v.first && region(u.second.region,v.second.region) && u.second.adjacent==v.second.adjacent;
        });
  };
  const auto owned=[](const auto& x,const auto& y,const auto& equal){
    return bool(x)==bool(y) && (!x || equal(*x,*y));
  };
  const auto domestic=[&](const NativeDomesticSources& x,const NativeDomesticSources& y){
    return x.straitAccess==y.straitAccess &&
        byKey(x.orders,y.orders,[](const auto& o){return std::pair{o.state,o.good};},[](const auto& u,const auto& v){return u==v;}) &&
        byKey(x.land,y.land,[](const auto& c){return c.id;},[](const auto& u,const auto& v){return u==v;}) &&
        byKey(x.builtRails,y.builtRails,[](const auto& c){return c.id;},[](const auto& u,const auto& v){return u==v;}) &&
        byKey(x.shipping.lanes,y.shipping.lanes,[](const auto& l){return l.id;},[](const auto& u,const auto& v){return u==v;});
  };
  return a.centers==b.centers &&
      byKey(a.states,b.states,[](const auto& x){return x.id;},[](const auto& x,const auto& y){return std::tie(x.id,x.region,x.market)==std::tie(y.id,y.region,y.market);}) &&
      amounts(a.statePairs,b.statePairs) && amounts(a.groupPairs,b.groupPairs) &&
      byKey(a.straits,b.straits,[](const auto& x){return x.id;},[](const auto& x,const auto& y){
        return std::tie(x.id,x.definition,x.seaProvinces,x.tariffModes,x.accessControls,x.canalDefinition)==
            std::tie(y.id,y.definition,y.seaProvinces,y.tariffModes,y.accessControls,y.canalDefinition);
      }) && byKey(a.groups,b.groups,[](const auto& x){return x.anchor;},[&](const auto& x,const auto& y){
        return x.anchor==y.anchor && byKey(x.states,y.states,[](auto id){return id;},[](auto u,auto v){return u==v;}) &&
            byKey(x.destinations,y.destinations,[](const auto& u){return u.state;},[&](const auto& u,const auto& v){
              return u.state==v.state && u.selected==v.selected &&
                  rows(u.candidates,v.candidates,[&](const auto& p,const auto& q){
                    return p.distanceFixed==q.distanceFixed && p.straits==q.straits &&
                        rows(p.provinces,q.provinces,[](const auto& first,const auto& second){
                          return first.first==second.first && first.last==second.last;
                        });
                  });
            });
      }) && owned(a.geometry,b.geometry,geometry) && owned(a.seaGeography,b.seaGeography,sea) && owned(a.domestic,b.domestic,domestic);
}
} // namespace v3

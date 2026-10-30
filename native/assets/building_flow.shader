Includes = {
    "cw/camera.fxh"
    "sharedconstants.fxh"
    "cw/heightmap.fxh"
    "jomini/jomini_water.fxh"
}

VertexStruct VS_INPUT
{
    float3 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
};
VertexStruct VS_OUTPUT
{
    float4 vPosition : PDX_POSITION;
    float2 vTexCoord : TEXCOORD0;
};

# Preserve CArrow's native buffer layout; the line renderer has another ABI.
ConstantBuffer( 0 )
{
    float AnimationLength;
    float MoveLength;
    float ClipLength;
    float TotalLength;
    float Width;
};
ConstantBuffer( 1 )
{
    int IsSelected;
    int IsFaded;
};

VertexShader =
{
    MainCode VertexShader
    {
        Input = "VS_INPUT"
        Output = "VS_OUTPUT"
        Code
        [[
            PDX_MAIN
            {
                VS_OUTPUT Out;
                float4 pos = float4( Input.vPosition, 1.0f );
                float height = max( GetHeight( pos.xz ), _WaterHeight );
                pos.y = lerp( height, _FlatmapHeight, _FlatmapLerp ) + 0.35f;
                Out.vPosition = FixProjectionAndMul( ViewProjectionMatrix, pos );
                Out.vTexCoord = Input.vTexCoord;
                return Out;
            }
        ]]
    }
}
PixelShader =
{
    TextureSampler DiffuseTexture
    {
        Index = 0
        MipFilter = "Linear"
        MinFilter = "Linear"
        MagFilter = "Linear"
        SampleModeU = "Wrap"
        SampleModeV = "Clamp"
    }
    TextureSampler MaskTexture
    {
        Index = 3
        MipFilter = "Linear"
        MinFilter = "Linear"
        MagFilter = "Linear"
        SampleModeU = "Clamp"
        SampleModeV = "Clamp"
    }
    MainCode PixelShader
    {
        Input = "VS_OUTPUT"
        Output = "PDX_COLOR"
        Code
        [[
            PDX_MAIN
            {
                // CArrow packs distance in V and ribbon width (0..1) in U.
                // Building-flow artwork instead points right along texture U.
                float2 uv = float2( Input.vTexCoord.y * ( 0.08f / max( Width, 0.01f ) ) - GlobalTime * 0.4f,
                                   Input.vTexCoord.x );
                float4 diffuse = PdxTex2D( DiffuseTexture, uv );
            #if defined( IRONROAD_BALANCED )
                float2 reverseUV = float2( -Input.vTexCoord.y * ( 0.08f / max( Width, 0.01f ) ) - GlobalTime * 0.4f, uv.y );
                diffuse = max( diffuse, PdxTex2D( DiffuseTexture, reverseUV ) );
            #endif
                float2 maskUV = float2( Input.vTexCoord.y * 0.03f, uv.y );
                float2 dx = ddx( maskUV );
                float2 dy = ddy( maskUV );
                float endU = max( TotalLength * 0.03f, 0.0001f );
                if ( endU < 1.0f )
                    maskUV.x += 1.0f - endU;
                else if ( maskUV.x > endU - 0.25f )
                    maskUV.x -= endU - 1.0f;
                else if ( maskUV.x > 0.25f )
                    maskUV.x = frac( ( maskUV.x - 0.25f ) / 0.5f ) * 0.5f + 0.25f;
                float4 mask = PdxTex2DGrad( MaskTexture, maskUV, dx, dy );
            #if defined( IRONROAD_TRAFFIC_GREEN )
                float3 tint = float3( 0.08f, 0.85f, 0.35f );
            #elif defined( IRONROAD_TRAFFIC_LIME )
                float3 tint = float3( 0.55f, 0.9f, 0.12f );
            #elif defined( IRONROAD_TRAFFIC_YELLOW )
                float3 tint = float3( 1.0f, 0.9f, 0.05f );
            #elif defined( IRONROAD_TRAFFIC_ORANGE )
                float3 tint = float3( 1.0f, 0.53f, 0.03f );
            #elif defined( IRONROAD_TRAFFIC_RED )
                float3 tint = float3( 0.95f, 0.04f, 0.12f );
            #elif defined( IRONROAD_INPUT )
                float3 tint = float3( 0.91f, 0.14924f, 0.0455f );
            #elif defined( IRONROAD_YELLOW )
                float3 tint = float3( 0.91f, 0.78f, 0.0455f );
            #elif defined( IRONROAD_BLUE )
                float3 tint = float3( 0.0455f, 0.35f, 0.91f );
            #else
                float3 tint = float3( 0.06426f, 0.51f, 0.0255f );
            #endif
                return float4( diffuse.rgb * tint * mask.rgb, diffuse.a * mask.a );
            }
        ]]
    }
}
BlendState BuildingFlowBlend
{
    BlendEnable = yes
    SourceBlend = "SRC_ALPHA"
    DestBlend = "INV_SRC_ALPHA"
}
RasterizerState BuildingFlowRasterizer
{
    CullMode = "none"
}
DepthStencilState BuildingFlowDepth
{
    DepthEnable = yes
    DepthWriteEnable = no
}
Effect IronroadBuildingOutput
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
}
Effect IronroadBuildingInput
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_INPUT" }
}
Effect IronroadBuildingYellow
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_YELLOW" }
}
Effect IronroadBuildingBlue
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_BLUE" }
}
Effect IronroadBuildingTrafficGreen
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_GREEN" }
}
Effect IronroadBuildingTrafficLime
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_LIME" }
}
Effect IronroadBuildingTrafficYellow
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_YELLOW" }
}
Effect IronroadBuildingTrafficOrange
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_ORANGE" }
}
Effect IronroadBuildingTrafficRed
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_RED" }
}

Effect IronroadBuildingOutputBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_BALANCED" }
}

Effect IronroadBuildingInputBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_INPUT" "IRONROAD_BALANCED" }
}

Effect IronroadBuildingTrafficGreenBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_GREEN" "IRONROAD_BALANCED" }
}

Effect IronroadBuildingTrafficLimeBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_LIME" "IRONROAD_BALANCED" }
}

Effect IronroadBuildingTrafficYellowBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_YELLOW" "IRONROAD_BALANCED" }
}

Effect IronroadBuildingTrafficOrangeBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_ORANGE" "IRONROAD_BALANCED" }
}

Effect IronroadBuildingTrafficRedBalanced
{
    VertexShader = "VertexShader"
    PixelShader = "PixelShader"
    BlendState = "BuildingFlowBlend"
    RasterizerState = "BuildingFlowRasterizer"
    DepthStencilState = "BuildingFlowDepth"
    Defines = { "IRONROAD_TRAFFIC_RED" "IRONROAD_BALANCED" }
}

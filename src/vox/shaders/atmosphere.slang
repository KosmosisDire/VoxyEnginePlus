#include "const.inl"

// Constants
static const float earthRadius = 6371000.0f;
static const float atmosphereHeight = 100000.0f;
static const float Hr = 7994.0f;                                 // Rayleigh scale height
static const float Hm = 1200.0f;                                 // Mie scale height
static const float3 betaR = float3(5.8e-6f, 13.5e-6f, 33.1e-6f); // Rayleigh scattering coefficients
static const float3 betaM = float3(21e-6f);                      // Mie scattering coefficient

// Struct to hold precomputed atmospheric parameters
struct AtmosphereParams
{
    float3 normalizedDir;
    float3 normalizedSunDir;
    float sunHeight;
    float viewHeight;
    float dayFactor;
    float sunsetFactor;
    float3 extinction;
    float opticalDepthR;
    float opticalDepthM;
    float cosTheta;
};

// Helper functions
float hash(float3 p)
{
    p = frac(p * float3(443.8975f, 397.2973f, 491.1871f));
    p += dot(p.xyz, p.yzx + 19.19f);
    return frac(p.x * p.y * p.z);
}

float getMiePhase(float cosTheta)
{
    const float g = 0.85f;
    float g2 = g * g;
    return (1.0f - g2) / (pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f) * 4.0f * PI);
}

float getRayleighPhase(float cosTheta)
{
    float cos2 = cosTheta * cosTheta;
    return (3.0f * (1.0f + cos2)) / (16.0f * PI);
}

float getOpticalDepth(float height, float H)
{
    float h = max(height - earthRadius, 0.0f);
    float density = exp(-h / H);
    float Chapman = sqrt(PI * H / 2.0f) * exp(-h / H);
    return density * Chapman;
}
// Initialize atmospheric parameters
AtmosphereParams initAtmosphere(float3 dir, float3 sunDir)
{
    AtmosphereParams params;

    params.normalizedDir = normalize(dir);
    params.normalizedSunDir = normalize(sunDir);
    params.sunHeight = params.normalizedSunDir.y;
    params.viewHeight = lerp(0.0f, atmosphereHeight, max(0.0f, params.normalizedDir.y));

    // Precompute common factors
    params.dayFactor = smoothstep(-0.05f, 0.15f, params.sunHeight);
    params.sunsetFactor = smoothstep(0.0f, 0.4f, params.sunHeight);

    // Calculate atmospheric extinction
    float atmosphereThickness = 1.0f / (max(params.sunHeight, 0.0f) + 0.15f);
    float3 rayleighSun = betaR * atmosphereThickness;
    float3 mieSun = betaM * atmosphereThickness;
    params.extinction = exp(-(rayleighSun + mieSun));

    // Precompute optical depths
    params.opticalDepthR = getOpticalDepth(params.viewHeight, Hr);
    params.opticalDepthM = getOpticalDepth(params.viewHeight, Hm);

    // Calculate view-sun angle
    params.cosTheta = dot(params.normalizedDir, params.normalizedSunDir);

    return params;
}

float3 getSunColor(AtmosphereParams params)
{
    float3 sunsetColor = float3(1.2f, 0.6f, 0.4f);
    float3 dayColor = float3(1.0f, 0.98f, 0.95f);

    float3 finalSunColor = lerp(sunsetColor, dayColor, params.sunsetFactor);
    finalSunColor *= params.extinction;

    return finalSunColor * params.dayFactor;
}

float3 getBaseSkyColor(AtmosphereParams params)
{
    float miePhase = getMiePhase(params.cosTheta);
    float rayleighPhase = getRayleighPhase(params.cosTheta);

    float3 rayleighScattering = betaR * params.opticalDepthR * rayleighPhase;
    float3 mieScattering = betaM * params.opticalDepthM * miePhase;

    float height = max(params.normalizedDir.y, 0.0f);
    float heightFactor = pow(1.0f - height, 4.0f);

    // Adjusted colors to match reference image
    float3 zenithColor = float3(0.2f, 0.4f, 1.0f);
    float3 horizonColor = float3(0.8f, 0.9f, 1.0f);
    float3 groundColor = float3(0.8f, 0.9f, 1.0f) * 0.2;

    float3 sunsetZenithColor = float3(0.3f, 0.2f, 0.7f);
    float3 sunsetHorizonColor = float3(1.9, 1, 0.2f);
    float3 sunsetGroundColor = float3(1, 0.5, 0.2f) * 0.2;

    float3 nightZenithColor = float3(0.1f, 0.1f, 0.5f) * 0.01;
    float3 nightHorizonColor = float3(0.1f, 0.1f, 0.1f) * 0.005;
    float3 nightGroundColor = float3(0.1f, 0.1f, 0.2f) * 0.005;

    // Calculate sun influence with below-horizon bloom
    float3 sunDir = params.normalizedSunDir;
    float3 viewDir = params.normalizedDir;
    float sunDist = length(viewDir - sunDir);
    float horizonDist = abs(viewDir.y);

    // Below horizon light scatter
    float belowHorizonFactor = exp(-horizonDist * 10.0f) * exp(-sunDist * 3.0f);
    float sunInfluence = exp(-sunDist * 5.0f) + belowHorizonFactor * 0.5f;

    // Sky color calculation
    float3 daySky = lerp(horizonColor, zenithColor, pow(height, 0.5f));
    float3 sunsetSky = lerp(sunsetHorizonColor, sunsetZenithColor, pow(height, 0.35f));
    float3 nightSky = lerp(nightHorizonColor, nightZenithColor, pow(height, 0.5f));
    float3 skyColor = lerp(sunsetSky, daySky, params.sunsetFactor);
    skyColor = lerp(nightSky, skyColor, params.dayFactor);

    // Improved multiple scattering
    float multiScatterFactor = 0.5f * (1.0f - exp(-params.opticalDepthR - params.opticalDepthM));
    float3 multiScatter = skyColor * multiScatterFactor;

    float3 currentSunColor = getSunColor(params);
    float3 scatteredLight = (rayleighScattering + mieScattering) * currentSunColor;
    float3 sky = skyColor * scatteredLight + multiScatter;

    // Sharp horizon transition with bloom
    float sharpness = 0.01f * (params.cosTheta + 2);
    float horizonBlend = smoothstep(-sharpness, sharpness * 2, viewDir.y);
    horizonBlend = pow(horizonBlend, 0.75f);

    // Ground color with sun influence
    float groundSunFactor = exp(-sunDist * 4.0f) * (1.0f - horizonBlend);
    float3 groundBlend = lerp(groundColor, sunsetGroundColor, 1 - params.sunsetFactor);
    groundBlend = lerp(groundBlend, nightGroundColor, 1 - params.dayFactor);
    groundBlend = lerp(groundBlend, currentSunColor * 0.2f, groundSunFactor * params.dayFactor);

    // Add below-horizon glow
    float3 horizonGlow = currentSunColor * belowHorizonFactor * params.dayFactor;
    groundBlend += horizonGlow * 0.2f;

    sky = lerp(groundBlend, sky, horizonBlend);

    return sky;
}

float3 getSunEffect(AtmosphereParams params)
{
    float3 currentSunColor = getSunColor(params);

    float sunDisk = smoothstep(0.9998f, 0.9999f, params.cosTheta);
    float sunCorona = pow(smoothstep(0.997f, 1.014, params.cosTheta) * 12, 3.0f) * 0.25f;
    float sunCorona2 = pow(smoothstep(0.93f, 1.13, params.cosTheta) * 4, 4.0f) * 0.25f;
    float sunGlow = pow(max(0.0f, params.cosTheta), 4.0f) * 0.2f;

    return (sunDisk * 50.0f + sunCorona + sunCorona2 + sunGlow) * currentSunColor;
}

float3 getStableStarCoord(float3 dir)
{
    float cells = 700.0f;
    float3 stableDir = floor(dir * cells) / cells;
    return normalize(stableDir);
}

float3 getStarEffect(AtmosphereParams params, float time)
{
    // Get stable star coordinates
    float3 stableDir = getStableStarCoord(params.normalizedDir);

    float starExistence = hash(stableDir * 1000.0f);
    if (starExistence >= 0.004f)
    {
        return float3(0.0002f);
    }

    float starProperties = hash(stableDir * 789.0f);
    float apparentMag = -1.0f + pow(starProperties, 0.2f) * 3.0f;
    float brightness = pow(2.512f, -apparentMag) * 0.5f;

    float twinkleFactor = lerp(0.2f, 0.4f, apparentMag / 6.0f);
    float twinkle = 1.0f + (hash(stableDir + frac(time * 0.15f)) * 2.0f - 1.0f) * twinkleFactor;

    float visibilityThreshold = lerp(0.1f, -0.15f, apparentMag / 6.0f);
    float visibilityFactor = smoothstep(visibilityThreshold, visibilityThreshold - 0.1f, params.sunHeight);

    float starIntensity = brightness * twinkle * visibilityFactor;

    float3 starColor = float3(1.0f,
                              lerp(1.0f, 0.9f + hash(stableDir * 123.456f) * 0.2f, apparentMag / 6.0f),
                              lerp(1.0f, 0.8f + hash(stableDir * 789.123f) * 0.3f, apparentMag / 6.0f));

    float3 stars = starColor * max(starIntensity, 0.0002f);

    // fade stars in ground
    float horizonFade = smoothstep(-0.1f, 0.5f, params.normalizedDir.y);
    stars *= horizonFade;

    return stars;
}

float3 getAtmosphereColor(float3 dir, float3 sunDir, float time)
{
    AtmosphereParams params = initAtmosphere(dir, sunDir);

    float3 sky = getBaseSkyColor(params);
    float3 sun = getSunEffect(params);
    float3 stars = getStarEffect(params, time);

    return sky + sun + stars;
}

float3 getAtmosphereColor(AtmosphereParams params, float time)
{
    float3 sky = getBaseSkyColor(params);
    float3 sun = getSunEffect(params);
    float3 stars = getStarEffect(params, time);

    return sky + sun + stars;

    // for debugging return day and night factor for every point of the screen
    // return float3(smoothstep(-0.05f, 0.15f, params.normalizedDir.y), smoothstep(0.0f, 0.4f, params.normalizedDir.y), 0);
}

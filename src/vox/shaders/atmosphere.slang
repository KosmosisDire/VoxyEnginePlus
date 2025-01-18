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
    const float g = 0.76f;
    float g2 = g * g;
    return (1.0f - g2) / pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f) / (4.0f * PI);
}

float getRayleighPhase(float cosTheta)
{
    return 3.0f * (1.0f + cosTheta * cosTheta) / (16.0f * PI);
}

float getOpticalDepth(float height, float H)
{
    return exp(-max(height - earthRadius, 0.0f) / H);
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
    params.dayFactor = smoothstep(-0.15f, 0.15f, params.sunHeight);
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
    float3 sunsetColor = float3(1.0f, 0.6f, 0.3f);
    float3 dayColor = float3(1.0f, 0.98f, 0.95f);

    float3 finalSunColor = lerp(sunsetColor, dayColor, params.sunsetFactor);
    finalSunColor *= params.extinction;

    float daylight = smoothstep(-0.1f, 0.15f, params.sunHeight);
    return finalSunColor * daylight;
}

float3 getBaseSkyColor(AtmosphereParams params)
{
    // Calculate scattering
    float miePhase = getMiePhase(params.cosTheta);
    float rayleighPhase = getRayleighPhase(params.cosTheta);

    float3 rayleighScattering = betaR * params.opticalDepthR * rayleighPhase;
    float3 mieScattering = betaM * params.opticalDepthM * miePhase;
    float3 totalScattering = rayleighScattering + mieScattering;

    // Get base colors
    float3 dayColor = float3(0.2f, 0.4f, 1.0f);
    float3 sunsetColor = float3(0.8f, 0.6f, 0.5f);
    float3 skyColor = lerp(sunsetColor, dayColor, params.sunsetFactor);

    // Multiple scattering approximation
    float multiScatterFactor = 0.5f * (1.0f - exp(-params.opticalDepthR - params.opticalDepthM));
    float3 multiScatter = skyColor * multiScatterFactor;

    // Combine effects with sun color
    float3 currentSunColor = getSunColor(params);
    float3 sky = skyColor * totalScattering * currentSunColor + multiScatter;

    // Apply day/night transition
    const float minNightLevel = 0.001f;
    sky = lerp(sky * max(0.02f, minNightLevel), sky, params.dayFactor);

    // Apply horizon effects
    float horizonBlend = smoothstep(-0.15f, 0.25f, params.normalizedDir.y);
    const float groundAlbedo = 0.1f;
    const float minGroundLevel = 0.0015f;

    // Calculate ground color with stronger sunset darkening
    float3 baseGroundColor = float3(0.7f, 0.75f, 0.8f);
    float sunsetDarkening = smoothstep(0.4f, -0.1f, params.sunHeight) * 0.85f;
    float3 groundColor = max(
        lerp(baseGroundColor, skyColor, groundAlbedo) * params.dayFactor * (1.0f - sunsetDarkening),
        float3(minGroundLevel));

    return lerp(groundColor, sky, horizonBlend);
}

float3 getSunEffect(AtmosphereParams params)
{
    float3 currentSunColor = getSunColor(params);

    float sunDisk = smoothstep(0.9995f, 0.9999f, params.cosTheta);
    float sunCorona = pow(max(0.0f, params.cosTheta), 32.0f) * 0.25f;
    float sunGlow = pow(max(0.0f, params.cosTheta), 4.0f) * 0.2f;

    return (sunDisk * 50.0f + sunCorona + sunGlow) * currentSunColor;
}

float3 getStableStarCoord(float3 dir)
{
    float cells = 750.0f;
    float3 stableDir = floor(dir * cells) / cells;
    return normalize(stableDir);
}

float3 getStarEffect(AtmosphereParams params, float time)
{
    // Get stable star coordinates
    float3 stableDir = getStableStarCoord(params.normalizedDir);

    float starExistence = hash(stableDir * 1000.0f);
    if (starExistence >= 0.007f)
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

    return starColor * max(starIntensity, 0.0002f);
}

float3 getAtmosphereColor(float3 dir, float3 sunDir, float time)
{
    AtmosphereParams params = initAtmosphere(dir, sunDir);

    float3 sky = getBaseSkyColor(params);
    float3 sun = getSunEffect(params);
    float3 stars = getStarEffect(params, time);

    return sky + sun + stars;
}

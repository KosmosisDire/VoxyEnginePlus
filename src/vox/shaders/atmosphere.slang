static const float earthRadius = 6371000.0f;
static const float atmosphereHeight = 100000.0f;
static const float Hr = 7994.0f;                                 // Rayleigh scale height
static const float Hm = 1200.0f;                                 // Mie scale height
static const float3 betaR = float3(5.8e-6f, 13.5e-6f, 33.1e-6f); // Rayleigh scattering coefficients
static const float3 betaM = float3(21e-6f);                      // Mie scattering coefficient

// Hash function for stable random values
float hash(float3 p)
{
    p = frac(p * float3(443.8975f, 397.2973f, 491.1871f));
    p += dot(p.xyz, p.yzx + 19.19f);
    return frac(p.x * p.y * p.z);
}

// Henyey-Greenstein phase function
float getMiePhase(float cosTheta, float g)
{
    float g2 = g * g;
    return (1.0f - g2) / pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f) / (4.0f * 3.14159f);
}

// Rayleigh phase function
float getRayleighPhase(float cosTheta)
{
    return 3.0f * (1.0f + cosTheta * cosTheta) / (16.0f * 3.14159f);
}

// Calculate optical depth for given height
float getOpticalDepth(float height, float H)
{
    return exp(-max(height - earthRadius, 0.0f) / H);
}

float3 getSunColor(float3 sunDir)
{
    float sunHeight = sunDir.y;
    float3 baseSunColor = float3(1.0f, 1.0f, 1.0f);

    // atmospheric extinction
    float atmosphereThickness = 1.0f / (max(sunHeight, 0.0f) + 0.15f);
    float3 rayleighSun = betaR * atmosphereThickness;
    float3 mieSun = betaM * atmosphereThickness;
    float3 extinction = exp(-(rayleighSun + mieSun));

    // Enhanced sunset colors
    float3 sunsetColor = float3(1.0f, 0.6f, 0.3f);
    float3 dayColor = float3(1.0f, 0.98f, 0.95f);
    float sunsetFactor = smoothstep(0.0f, 0.4f, sunHeight);
    float3 finalSunColor = lerp(sunsetColor, dayColor, sunsetFactor);
    finalSunColor *= extinction;

    // day/night transition
    float daylight = smoothstep(-0.1f, 0.15f, sunHeight);
    return finalSunColor * daylight;
}

float3 getBaseSkyColor(float3 dir, float3 sunDir)
{
    float3 nDir = normalize(dir);
    float3 nSunDir = normalize(sunDir);
    float sunHeight = nSunDir.y;
    float height = nDir.y;

    // Calculate view ray parameters
    float viewHeight = lerp(0.0f, atmosphereHeight, max(0.0f, height));
    float viewDist = sqrt(max(0.0f, atmosphereHeight * atmosphereHeight - viewHeight * viewHeight));

    // Calculate optical depths
    float opticalDepthR = getOpticalDepth(viewHeight, Hr);
    float opticalDepthM = getOpticalDepth(viewHeight, Hm);

    // Get scattering angles
    float cosTheta = dot(nDir, nSunDir);
    float miePhase = getMiePhase(cosTheta, 0.76f);
    float rayleighPhase = getRayleighPhase(cosTheta);

    // Calculate sun contribution
    float3 currentSunColor = getSunColor(nSunDir);

    // scattering calculation
    float3 rayleighScattering = betaR * opticalDepthR * rayleighPhase;
    float3 mieScattering = betaM * opticalDepthM * miePhase;
    float3 totalScattering = rayleighScattering + mieScattering;

    // Enhanced sky colors
    float3 dayColor = float3(0.2f, 0.4f, 1.0f);
    float3 sunsetColor = float3(0.8f, 0.6f, 0.5f);
    float sunsetFactor = smoothstep(-0.1f, 0.4f, sunHeight);
    float3 skyColor = lerp(sunsetColor, dayColor, sunsetFactor);

    // multiple scattering approximation
    float multiScatterFactor = 0.5f * (1.0f - exp(-opticalDepthR - opticalDepthM));
    float3 multiScatter = skyColor * multiScatterFactor;

    // Combine all effects
    float3 sky = skyColor * totalScattering * currentSunColor + multiScatter;

    // Enhanced day/night transition with minimum light level
    float dayFactor = smoothstep(-0.15f, 0.15f, sunHeight);
    float minNightLevel = 0.001f; // Ensure we never hit pure black
    sky = lerp(sky * max(0.02f, minNightLevel), sky, dayFactor);

    // horizon effects with smoother transition
    float horizonBlend = smoothstep(-0.15f, 0.25f, height);
    float groundAlbedo = 0.1f;      // Approximate ground reflection
    float minGroundLevel = 0.0015f; // Slightly higher minimum for ground
    float3 groundColor = max(
        lerp(
            float3(0.7f, 0.75f, 0.8f),
            skyColor,
            groundAlbedo) *
            dayFactor,
        float3(minGroundLevel));

    sky = lerp(
        groundColor,
        sky,
        horizonBlend);

    return sky;
}

float3 getSunEffect(float3 dir, float3 sunDir)
{
    float3 nDir = normalize(dir);
    float3 nSunDir = normalize(sunDir);
    float cosTheta = dot(nDir, nSunDir);

    float3 currentSunColor = getSunColor(nSunDir);

    // sun disk and corona
    float sunDisk = smoothstep(0.9995f, 0.9999f, cosTheta);
    float sunCorona = pow(max(0.0f, cosTheta), 32.0f) * 0.25f;
    float sunGlow = pow(max(0.0f, cosTheta), 4.0f) * 0.2f;

    return (sunDisk * 50.0f + sunCorona + sunGlow) * currentSunColor;
}

// Stars stable coordinates
float3 getStableStarCoord(float3 dir)
{
    float cells = 750.0f;
    float3 stableDir = floor(dir * cells) / cells;
    return normalize(stableDir);
}

float3 getStarEffect(float3 dir, float3 sunDir, float time)
{
    float3 nDir = normalize(dir);
    float3 nSunDir = normalize(sunDir);
    float sunHeight = nSunDir.y;

    // Get stable star coordinates
    float3 stableDir = getStableStarCoord(nDir);

    // Generate star properties using hash
    float starExistence = hash(stableDir * 1000.0f);
    float starProperties = hash(stableDir * 789.0f);

    // Only process if this could be a star (optimization)
    if (starExistence < 0.007f)
    {
        // Generate apparent magnitude (-1 to 6)
        // Brighter stars are rarer using non-linear distribution
        float apparentMag = -1.0f + pow(starProperties, 0.2f) * 3.0f;

        // Calculate base brightness from magnitude
        // Each magnitude step is 2.512 times dimmer
        float brightness = pow(2.512f, -apparentMag) * 0.5f;

        // Enhanced twinkling effect scaled by magnitude
        // Brighter stars twinkle less
        float twinkleFactor = lerp(0.2f, 0.4f, apparentMag / 6.0f);
        float twinkle = 1.0f + (hash(stableDir + frac(time * 0.15f)) * 2.0f - 1.0f) * twinkleFactor;

        // Progressive appearance based on magnitude
        // Brighter stars appear earlier in twilight
        float visibilityThreshold = lerp(0.1f, -0.15f, apparentMag / 6.0f);
        float visibilityFactor = smoothstep(visibilityThreshold, visibilityThreshold - 0.1f, sunHeight);

        // Calculate final star intensity
        float starIntensity = brightness * twinkle * visibilityFactor;

        // Add slight color variation for brighter stars
        float3 starColor = float3(1.0f,
                                  lerp(1.0f, 0.9f + hash(stableDir * 123.456f) * 0.2f, apparentMag / 6.0f),
                                  lerp(1.0f, 0.8f + hash(stableDir * 789.123f) * 0.3f, apparentMag / 6.0f));

        return starColor * max(starIntensity, 0.0002f);
    }

    return float3(0.0002f); // Minimum night sky brightness
}

float3 getAtmosphereColor(float3 dir, float3 sunDir, float time)
{
    float3 sky = getBaseSkyColor(dir, sunDir);
    float3 sun = getSunEffect(dir, sunDir);
    float3 stars = getStarEffect(dir, sunDir, time);

    return sky + sun + stars;
}

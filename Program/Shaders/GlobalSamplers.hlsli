SamplerState g_pointWrap : register(s0); // No interpolation, Point Wrap
SamplerState g_linearWrap : register(s1); // Trilinear interpolation, Linear Wrap
SamplerState g_anisotropicWrap : register(s2); // Anisotropic filtering, Wrap
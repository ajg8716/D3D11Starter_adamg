Assignment 13 – Final Assignment
=================================

------------------------------------------------------------------------
Task 1: Post-Process Gaussian Blur
------------------------------------------------------------------------
Shader:  BlurPS.hlsl
VS:      PostProcessVS.hlsl  (shared with Task 2)

Implementation:
  The scene (entities + sky) is rendered into an off-screen Texture2D
  (ppRTV / ppSRV) instead of the back buffer.  After all 3-D drawing is
  complete, BlurPS runs a 2-D Gaussian-weighted kernel over that texture.

  The kernel loops from -blurRadius to +blurRadius in both X and Y.
  Each sample is offset by (x,y) * texelSize from the current pixel.
  The weight for each sample is:

      w = exp( -dist² / (2 * sigma²) )   where sigma = blurRadius / 2

  All weighted samples are summed and divided by the total weight to
  produce a normalised output colour.

  When blurRadius == 0 the shader takes the early-out path and returns
  the centre sample unchanged, so there is zero visual or GPU cost.

  The ImGui "Post Process" window contains a slider (range 0–10).

  ImGui is rendered AFTER the post-process pass writes to the back buffer,
  so the UI is never blurred.

C++ additions in Game.h / Game.cpp:
  - BlurCB struct (16-byte aligned)
  - ppRTV / ppSRV / ppTexture  (off-screen render target)
  - ppPingRTV / ppPingSRV / ppPingTexture  (ping-pong for two-pass chain)
  - ppSampler  (clamp sampler, shared by all PP passes)
  - ppVS / blurPS  (loaded from PostProcessVS.cso / BlurPS.cso)
  - CreatePostProcessResources()
  - RunPostProcessPass()  (generic single-pass helper)
  - blurEnabled / blurRadius  (ImGui state)
  - OnResize() recreates the two render targets at the new window size

------------------------------------------------------------------------
Task 2: Chromatic Aberration
------------------------------------------------------------------------
Shader:  ChromaticAberrationPS.hlsl
VS:      PostProcessVS.hlsl  (shared with Task 1)

What it is:
  Chromatic aberration is a lens optical artefact where different
  wavelengths of light refract by slightly different amounts, causing
  red, green, and blue to appear offset from each other.  The effect
  is strongest at screen corners and zero at the exact centre.

Implementation:
  A direction vector is computed from the screen centre to the current
  pixel:  dir = uv – (0.5, 0.5)
  Each channel is sampled at a different UV:
      R  →  uv + dir * strength   (shifted outward)
      G  →  uv                    (no shift)
      B  →  uv – dir * strength   (shifted inward)
  Because dir grows larger toward the corners, the colour fringing is
  strongest there, matching real lens dispersion.

  When strength == 0 the shader returns the unmodified sample immediately.

  The ImGui "Post Process" window contains a slider (range 0.000–0.030).

Pipeline order when both effects are active:
  scene (ppRTV) → Blur pass → ppPingRTV → Chroma pass → back buffer

  Each pass is driven by Game::RunPostProcessPass(), which:
    - Binds the destination RTV (no depth buffer)
    - Sets a full-window viewport
    - Uploads the CB data through FillAndBindNextConstantBuffer()
    - Sets PostProcessVS + the supplied PS
    - Nulls the input layout (SV_VertexID trick, no vertex buffer)
    - Calls Draw(3, 0) for the full-screen triangle

C++ additions:
  - ChromaCB struct
  - chromaPS  (loaded from ChromaticAberrationPS.cso)
  - chromaEnabled / chromaStrength  (ImGui state)

------------------------------------------------------------------------
Notes
------------------------------------------------------------------------
1 - Error
ID3D11DeviceContext::DrawIndexed: The Vertex Shader expects application provided input data (which is to say data other than hardware auto-generated values such as VertexID or InstanceID). Therefore an Input Assembler object is expected, but none is bound.
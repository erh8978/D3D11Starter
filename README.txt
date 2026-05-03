I initially wanted to try SSAO, so I set up multiple render targets and got that working.
However, once I got to that point, I realized SSAO would take WAY more time than I had left, so I pivoted to doing 'god rays' as a post-process.
So I added a sun visibility render target, and got both the skybox and regular PS to output to the correct targets.
The pivot is why I have render targets for normals and depth as well, despite not using them.
I don't have ImGui set up in my D3D12 branch, so all the light ray settings are hardcoded in SunRays.PS, but the current settings should be fairly good.
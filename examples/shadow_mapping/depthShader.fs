#version 410 core

void main()
{
  // because we don't need color buffer in the first pass so we bypass here
  // doesn't need to explict assign depth here, because this is what happen in backgroung anyway
  // gl_FragDepth = gl_FragCoord.z;
}
local target_name = 'Core'

raven_engine_target(target_name)
  add_packages('mimalloc')
  add_packages('glm', 'tracy', { public = true })
  
  add_defines('GLM_FORCE_DEPTH_ZERO_TO_ONE=', 'GLM_FORCE_XYZW_ONLY=', 'GLM_FORCE_EXPLICIT_CTOR=', { public = true })
raven_end_target()

raven_test_target(target_name .. '_Tests')
  add_deps(target_name)
raven_end_target()
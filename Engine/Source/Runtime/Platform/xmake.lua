local target_name = 'Platform'

raven_engine_target(target_name)
  add_deps('Core')

  add_packages('glfw')
raven_end_target()
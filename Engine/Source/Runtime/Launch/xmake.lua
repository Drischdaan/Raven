local target_name = 'Launch'

raven_launch_target(target_name)
  add_deps('Core', 'Platform', 'Engine')
raven_end_target()
local target_name = 'Engine'

raven_engine_target(target_name)
   add_deps('Core', 'Platform')
raven_end_target()
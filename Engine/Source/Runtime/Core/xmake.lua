local target_name = 'Core'

raven_engine_target(target_name)
  add_packages('mimalloc')
raven_end_target()

raven_test_target(target_name .. '_Tests')
  add_deps(target_name)
raven_end_target()
local target_name = 'Raven'

raven_engine_target(target_name)
  add_packages('directx12-agility')
  add_packages('directx-headers')
  add_packages('directxshadercompiler')

  add_links('d3d12.lib', 'dxgi.lib', 'dxguid.lib')
raven_target_end()
function raven_target(name, group)
  target(name)
    set_group(group)
end

function raven_end_target()
  target_end()
end

function raven_add_default_files()
  add_files('Private/**.cpp')
  add_headerfiles('Public/**.hpp', 'Private/**.hpp')

  add_includedirs('Private')
  add_includedirs('Public', {public = true})
end

function raven_engine_target(name)
  raven_target(name, 'Engine')
    if is_mode('debug') or is_mode('development') then
      set_kind('shared')

      if is_plat('windows') then
        add_defines(string.upper(name) .. '_API=__declspec(dllexport)')
        add_defines(string.upper(name) .. '_API=__declspec(dllimport)', {interface = true})
      end
    else
      set_kind('static')
      add_defines(string.upper(name) .. '_API=', { public = true })
    end

    raven_add_default_files()

    if is_plat('windows') then
      add_rules('raven.windows.default')
    end
end

function raven_launch_target(name)
  raven_target(name, 'Engine')
    set_kind('binary')

    raven_add_default_files()

    if is_plat('windows') then
      add_rules('raven.windows.default')
    end
end

function raven_test_target(name)
  raven_target(name, 'Tests')
    set_kind('binary')

    add_files('Test/**.cpp')
    add_headerfiles('Test/**.hpp')

    add_includedirs('Test')

    if is_plat('windows') then
      add_rules('raven.windows.default')
    end

    add_packages('snitch')
end
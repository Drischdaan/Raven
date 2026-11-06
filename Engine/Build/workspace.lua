function setup_workspace()
  set_allowedplats('windows')
  set_allowedarchs('x64')
  set_allowedmodes('debug', 'development', 'shipping')

  set_defaultplat('windows')
  set_defaultarchs('x64')
  set_defaultmode('development')

  set_rundir('./')

  set_languages('cxx23')

  set_targetdir('$(projectdir)/Binaries/Engine/$(plat)-$(arch)/$(mode)')
  set_objectdir('$(projectdir)/Intermediates/Engine/$(plat)-$(arch)/$(mode)')
  set_dependir('$(projectdir)/Engine/Build/Output/.deps')

  set_warnings('error')
  set_exceptions('none')

  if is_mode('debug') then
    set_symbols('debug')
    set_optimize('none')
    set_runtimes('MDd')
    add_defines('RAVEN_BUILD_MODULAR')
    add_defines('RAVEN_MODE_DEBUG', 'RAVEN_MODE="Debug"')
  elseif is_mode('development') then
    set_symbols('debug')
    set_optimize('fastest')
    set_strip('all')
    set_runtimes('MDd')
    add_defines('RAVEN_BUILD_MODULAR')
    add_defines('RAVEN_MODE_DEVELOPMENT', 'RAVEN_MODE="Development"')
  elseif is_mode('shipping') then
    set_symbols('debug')
    set_optimize('aggressive')
    set_strip('all')
    set_runtimes('MD')
    add_defines('RAVEN_BUILD_MONOLITHIC')
    add_defines('RAVEN_MODE_SHIPPING', 'RAVEN_MODE="Shipping"')
  end

  if is_plat('windows') then
    add_cxflags('/EHsc', '/Zc:preprocessor', '/wd5103', '/wd4005', {force = true})
    add_defines('WIN32_LEAN_AND_MEAN', 'NOMINMAX', 'WIN32_DEFAULT_LIBS')
  end

  rule('raven.windows.default')
    after_load(function (target)
      target:add('syslinks', 'kernel32', 'user32', 'gdi32', 'winspool', 'comdlg32', 'advapi32')
      target:add('syslinks', 'shell32', 'ole32', 'oleaut32', 'uuid', 'odbc32', 'odbccp32', 'comctl32')
      target:add('syslinks', 'comdlg32', 'setupapi', 'shlwapi')
      if not target:is_plat('mingw') then
        target:add('syslinks', 'strsafe')
      end
    end)
  rule_end()
end

function raven_target(name, group)
  target(name)
    set_group(group)

    after_build(function (target)
      for _, pkg in pairs(target:pkgs()) do
        local bindir = path.join(pkg:installdir(), 'bin')
        if os.isdir(bindir) then
          os.trycp(path.join(bindir, '*.dll'), target:targetdir())
        end
      end
    end)
end

function raven_target_end()
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
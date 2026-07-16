set_xmakever('3.0.9')

-- Project Configuration
set_project('Raven')
set_description('Raven Engine')
set_version('0.0.0', { build = '%Y%m%d%H%M' })

set_allowedplats('windows', 'linux')
set_defaultplat('windows')

set_allowedarchs('x64')
set_defaultarchs('x64')

set_allowedmodes('debug', 'development', 'shipping')
set_defaultmode('development')

set_rundir('.')

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
  
  add_defines('RAVEN_ENABLE_ASSERTS=1', 'RAVEN_PROFILER_ENABLED=1', 'TRACY_ENABLE')
elseif is_mode('development') then
  set_symbols('debug')
  set_optimize('fastest')
  set_strip('all')
  set_runtimes('MDd')
  add_defines('RAVEN_BUILD_MODULAR')
  add_defines('RAVEN_MODE_DEVELOPMENT', 'RAVEN_MODE="Development"')

  add_defines('RAVEN_ENABLE_ASSERTS=0', 'RAVEN_PROFILER_ENABLED=1', 'TRACY_ENABLE')
elseif is_mode('shipping') then
  set_symbols('debug')
  set_optimize('aggressive')
  set_strip('all')
  set_runtimes('MD')
  add_defines('RAVEN_BUILD_MONOLITHIC')
  add_defines('RAVEN_MODE_SHIPPING', 'RAVEN_MODE="Shipping"')

  add_defines('RAVEN_ENABLE_ASSERTS=0', 'RAVEN_PROFILER_ENABLED=1', 'TRACY_ENABLE')
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

-- Dependencies
add_requires("snitch 1.3.2")
add_requires('glfw 3.4')
add_requires('mimalloc 3.3.2', { configs = { cxx = true } })
add_requires('glm 1.0.3', { configs = { cxx_standard = '20' } })
add_requires('tracy 0.13.0', { configs = { shared = true } })

-- Helper Functions
includes('Engine/Build/targets.lua')

-- Targets
includes('Engine/**/xmake.lua')
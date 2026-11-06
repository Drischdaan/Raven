set_xmakever('3.1.2')

set_project('Raven')
set_description('A modern c++ game engine')
set_version('0.0.0', { build = '%Y%m%d%H%M' })

includes('Engine/Build/workspace.lua')

setup_workspace()

includes('Engine/Source/**/xmake.lua')
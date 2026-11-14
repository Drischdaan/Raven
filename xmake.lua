set_xmakever('3.1.2')

set_project('Raven')
set_description('A modern c++ game engine')
set_version('0.0.0', { build = '%Y%m%d%H%M' })

includes('Engine/Build/workspace.lua')

setup_workspace()

-- Dependencies START
add_requires('directx12-agility 1.619.2')
add_requires('directx-headers 1.619.5')
add_requires('directxshadercompiler 1.8.2405')

-- Dependencies END

includes('Engine/Source/**/xmake.lua')
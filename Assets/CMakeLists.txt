cmake_minimum_required (VERSION 3.20)

project (
   "Assets"
   VERSION 0.1
   DESCRIPTION "Pikzel Shared Assets"
)

file(GLOB_RECURSE BackpackModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Backpack/*)
file(GLOB_RECURSE SponzaModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Sponza/*)
file(GLOB_RECURSE Fonts RELATIVE ${PROJECT_SOURCE_DIR} Fonts/*)
file(GLOB_RECURSE Skyboxes RELATIVE ${PROJECT_SOURCE_DIR} Skyboxes/*)

copy_assets(BackpackModel Models "Assets/Models" CopiedBackpackModel)
copy_assets(SponzaModel Models "Assets/Models" CopiedSponzaModel)
copy_assets(Fonts Models Assets/Fonts CopiedFonts)
copy_assets(Skyboxes Models Assets/Skyboxes CopiedSkyboxes)

source_group("Models/Backpack" FILES ${BackpackModel})
source_group("Models/Sponza" FILES ${SponzaModel})
source_group("Fonts" FILES ${Fonts})
source_group("Skyboxes" FILES ${Skyboxes})

add_custom_target(
   ${PROJECT_NAME}
   SOURCES ${BackpackModel} ${SponzaModel} ${Fonts} ${Skyboxes}
   DEPENDS ${CopiedBackpackModel} ${CopiedSponzaModel} ${CopiedFonts} ${CopiedSkyboxes}
)

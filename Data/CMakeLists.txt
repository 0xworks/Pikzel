cmake_minimum_required (VERSION 3.16)

project (
   "Data"
   VERSION 0.1
   DESCRIPTION "Pikzel Shared Data"
)

file(GLOB BackpackModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Backpack/*)
file(GLOB SponzaModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Sponza/*)
file(GLOB Fonts RELATIVE ${PROJECT_SOURCE_DIR} Fonts/*)
file(GLOB Skyboxes RELATIVE ${PROJECT_SOURCE_DIR} Skyboxes/*)

copy_assets(BackpackModel Assets/Models/Backpack CopiedBackpackModel)
copy_assets(SponzaModel Assets/Models/Sponza CopiedSponzaModel)
copy_assets(Fonts Assets/Fonts CopiedFonts)
copy_assets(Skyboxes Assets/Skyboxes CopiedSkyboxes)

source_group("Models/Backpack" FILES ${BackpackModel})
source_group("Models/Sponza" FILES ${SponzaModel})
source_group("Fonts" FILES ${Fonts})
source_group("Skyboxes" FILES ${Skyboxes})

add_custom_target(
   ${PROJECT_NAME}
   SOURCES ${BackpackModel} ${SponzaModel} ${Fonts} ${Skyboxes}
   DEPENDS ${CopiedBackpackModel} ${CopiedSponzaModel} ${CopiedFonts} ${CopiedSkyboxes}
)

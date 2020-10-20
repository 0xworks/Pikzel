cmake_minimum_required (VERSION 3.16)

project (
   "Data"
   VERSION 0.1
   DESCRIPTION "Pikzel Shared Data"
)

file(GLOB BackpackModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Backpack/*)
file(GLOB SponzaModel RELATIVE ${PROJECT_SOURCE_DIR} Models/Sponza/*)

copy_assets(BackpackModel Models/Backpack CopiedBackpackModel)
copy_assets(SponzaModel Models/Sponza CopiedSponzaModel)

source_group("Models/Backpack" FILES ${BackpackModel})
source_group("Models/Sponza" FILES ${SponzaModel})

add_custom_target(
   ${PROJECT_NAME}
   SOURCES ${BackpackModel} ${SponzaModel}
   DEPENDS ${CopiedBackpackModel} ${CopiedSponzaModel}
)

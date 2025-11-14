# Build the app, using the HAL

include_directories(include)
set(PROJECT_NAME "as2_d")
file(GLOB MY_SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE hal)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE gpiod)
target_link_libraries(${PROJECT_NAME}  PRIVATE m)
# Copy executable to final location (change `a1` to project name as needed)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:${PROJECT_NAME}>"
     "~/ENSC351/public/myApps/${PROJECT_NAME}" 
  COMMENT "Copying ARM executable to public NFS directory")
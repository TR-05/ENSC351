# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(ledTest ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(ledTest LINK_PRIVATE hal)
target_link_libraries(ledTest LINK_PRIVATE gpiod)

# Copy executable to final location (change `ledTest` to project name as needed)
add_custom_command(TARGET ledTest POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:ledTest>"
     "~/ENSC351/public/myApps/ledTest" 
  COMMENT "Copying ARM executable to public NFS directory")
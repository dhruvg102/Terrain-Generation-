
if exist build\ (
  cd build 
) else (
  mkdir build
  cd build
)

call cmake ..
PAUSE

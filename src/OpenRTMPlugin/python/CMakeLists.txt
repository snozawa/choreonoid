
# @author Shin'ichiro Nakaoka

set(target CnoidPyOpenRTMPlugin)
add_cnoid_python_module(${target} PyOpenRTMPlugin.cpp)
target_link_libraries(${target} CnoidOpenRTMPlugin ${PYTHON_LIBRARIES} ${Boost_PYTHON_LIBRARY})

Q1: They are billboarding because the Projection Matrix faces the camera. By multiply the position by the camera, it controls the distance in the eye space from the camera (z).
Q2: Due to the ordering of how the vertices were set and sent into the shaders, the clip space does not adapt to the orientation of the camera, therefore the quads initially in the back will retain its black squares as it is rendered before the initially in-front squares.
Q3: The vertices and attributes have already been loaded into the GPU.
Q4: The objects do not adapt to the orientation of the camera, therefore, based on the initial rendering the first objects retain the black square versus the later objects.
Q5: No, its still unclear on application and usage and when/where.
Q6: 3.
Q7: The lab was good.
Q8: 5 hrs.
Q9: None.
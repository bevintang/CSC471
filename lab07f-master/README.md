
CSC 471 -- Program 2B: Snow Globe -- Leg Day Duck

Student: Bevin Tang
Date: Oct. 18, 2017

** Description: 
The files animate a running duck inside of a snowglobe -- complete with smalling snow. Using the A and D keys, the user can rotate the globe and its contents either left or right.
**

** Resources:
Sphere.obj aquired online at:
https://github.com/libgdx/libgdx/blob/master/tests/gdx-tests-android/assets/data/sphere.obj
**

** Code Compromises: 
I encountered a bug where I could not access any new functions I created inside of the class. So, due to time constaints, I made two functions, as well as three variables, global so that I may use them inside the class.

Due to the time constraints (again), I could not think of a nice way to get the snow to appear more scattered, so I opted to use randomly generated floats inside of large arrays holding the x-position and the z-position.
**
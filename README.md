# cs-330
Comp Graphic and Visualization

# Project Reflection: 3D Workstation Scene (OpenGL)

## Resources Used

* C++
* OpenGL
* Visual Studio
* GLFW
* GLEW
* GLM Mathematics Library
* Texture Mapping
* Phong Lighting Model

## Overview

During this course, I designed and developed a three-dimensional workstation scene using C++ and OpenGL. The project was completed over several milestones that gradually introduced object modeling, transformations, textures, lighting, camera controls, and scene management before combining them into a completed interactive environment. The final scene was loosely based on a photograph of my personal computer desk and includes a monitor, keyboard, mouse, headset and stand, Nintendo Switch with controller, iPad, and supporting furniture. The actual scene has so many objects that the scope of the project would unfortunately have been too large to complete satisfactorily for the time frame students were allotted, so a scaled down version of the scene was created, primarily focusing on the objects on the top of the desk.

This project provided practical experience with the complete graphics development process, beginning with simple geometric primitives and progressing into a fully interactive scene with detailed textures and lighting that could be explored through user-controlled camera movement.

---

## Work Completed

The project began with constructing objects from low-polygon geometric primitives including boxes, cylinders, spheres, and planes. As each milestone was completed, additional functionality was introduced through texture mapping, lighting, material properties, and camera navigation.

Several textures were created from screenshots of my own devices, including my Windows desktop, Nintendo Switch home screen, and iPad home screen, allowing the finished scene to more closely resemble my actual workstation. Lighting was expanded to include multiple light sources using the Phong lighting model, while camera controls allowed movement throughout the scene using both keyboard and mouse input.

Throughout development I focused on creating recognizable objects while maintaining relatively simple geometry, demonstrating that realism can often be achieved through thoughtful placement, lighting, and textures rather than highly detailed models with more complex shapes and higher polygon counts.

---

## Software Design Process

One of the most valuable lessons from this project was learning how important planning is before writing code. Rather than attempting to recreate each object exactly as it appeared in the reference photograph, I first analyzed each object to determine which primitive shapes could be combined to approximate it. This greatly simplified the modeling process while keeping polygon counts low.

The design process followed throughout the course was highly iterative. Basic geometry was created first, followed by repeated refinement of object dimensions, positioning, textures, materials, lighting, and camera placement. As the scene became more complete, smaller adjustments often produced significant improvements in realism. This iterative approach reinforced the importance of testing frequently and refining software over multiple development cycles rather than expecting an ideal solution during the initial implementation.

The same design philosophy can easily be applied to future software projects by establishing a solid foundation first, testing regularly, and making incremental improvements as requirements evolve.

---

## Software Development Process

My overall approach to software development throughout this project centered on implementing one feature at a time and verifying its functionality before introducing additional complexity. This greatly reduced the number of problems encountered during development and made debugging significantly more manageable.

As the project grew larger, I began organizing the code into reusable functions responsible for transformations, texture loading, material selection, lighting configuration, and object creation. Separating responsibilities into logical functions improved readability while making future modifications considerably easier.

Compared to the beginning of the course, I found myself thinking much more about maintainability, organization, and readability instead of simply completing individual programming tasks. By the final milestone, I was approaching the assignment much more like a complete software project than a collection of individual exercises.

---

## Growth in Understanding Computer Science

Although my long-term career interests are centered around systems administration and information technology rather than graphics programming at this time, this course significantly expanded my understanding of software development, and gave me some possible inspiration for some side projects outside of day-to-day work. Before taking this course, I had very little experience with computational graphics. By the conclusion of the project, I had developed a practical understanding of object transformations, coordinate systems, texture mapping, lighting models, camera movement, and scene management within OpenGL.

Perhaps more importantly, manually constructing a three-dimensional scene gave me a much greater appreciation for professional graphics software such as Blender, Maya, and ZBrush. Working directly with transformations, matrices, and object placement provided valuable insight into the mathematical operations that those applications perform behind the scenes.

---

## Future Applications

The knowledge gained throughout this course extends beyond computer graphics alone. The iterative development strategies, modular programming techniques, debugging practices, and structured problem-solving approach used throughout this project can all be applied to future software development work.

From an educational perspective, this course strengthened my understanding of mathematical concepts commonly used throughout computer science, including vectors, transformations, and geometric reasoning. Professionally, while I may not pursue a career in graphics programming, these experiences have improved my overall confidence as a software developer and strengthened skills that will continue to benefit me as I transition into systems administration and information technology. Additionally, this project has inspired me to continue exploring graphics programming and game development as personal learning projects in the future.

---

## Portfolio Artifact

This repository includes:

* The completed **3D Scene** project archive.
* The accompanying **Design Decisions** document.
* This README reflection.

Together, these artifacts demonstrate my ability to design, develop, and refine a complete interactive three-dimensional environment while applying computational graphics concepts, OpenGL programming techniques, and modern software development practices.

---

## Conclusion

This project represents a significant milestone in my computer science education by bringing together mathematical concepts, programming techniques, and software design principles into a single application. Developing the workstation scene demonstrated that effective graphics programming is not simply about creating visually appealing objects, but about carefully planning, iterating, testing, and refining software over time.

Although I do not anticipate specializing in graphics programming professionally, the experience gained throughout this course has strengthened my overall software development skills while providing a greater appreciation for the technology behind modern visualization software, interactive applications, and computer-generated environments. This project serves as both a demonstration of the technical skills developed during CS-330 and a reminder of how much my approach to software design and development has evolved throughout the course.

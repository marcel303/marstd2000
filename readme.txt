MARSTD/MARX README FILE
-----------------------

[See the documentation for more information]


Installing:

  Copy the directory ./marstd/ to your include/ directory.
  The directory ./marstd/ includes all headers and sources to use MARSTD and MARX.
  That's all.


Using:

  A> #include <marstd/marstd.h>
  B> #include <marstd/marstd.cpp>

  A> Add this line to every source file that will be using MARSTD.
  B> Add this line exactly once in your entire project.


  C> #include <marstd/marx.h>
  D> #include <marstd/marx.cpp>

  C> Add to above line to every source file that will be using MARX.
  D> Add to above line exactly once in every project that will be using MARX.


  The implementation of MARSTD is for a large part inside the inline/header files. Some
  classes have code in the marstd.cpp file though. To use these classes, you will
  need to include marstd.cpp too. Be carefull to include this file only once though.
  Like said before some classes do not require you to include the marstd.cpp file.
  It is best not to rely on this, since it may change in the future. It is best to
  always include the .cpp file.

  The same is true for the marx.cpp file. The _entire_ implementation of MARX is
  inside the .cpp file though, so every time you'll use MARX, you'll need to include
  the .cpp file.

  It may be wise to include the .cpp files inside one of your source files that is
  rarely compiled, or a seperate file for MARSTD/MARX only. Eg:

  <!-- contents of marinc.cpp

    #include <marstd/marstd.cpp>
    #include <marstd/marx.cpp>

  end of marinc.cpp --!>


Removing:

  Simply delete the directory include/marstd/.


Updating:

  Copy the contents of an updated MARSTD/MARX librairy to include/marstd/ and
  overwrite all files. Make sure to compile older projects using 'rebuild all'.


Compiling:

  Compiling the test program can be done using Dev-C++ or RHIDE. If you will be using Dev-C++,
  just open the .dev project file and run. If you are using RHIDE, open the appropiate .gpr file.
  and press F9.

  See the readme.txt file in the examples/ directory for information on compiling the examples.

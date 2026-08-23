cl /std:c++latest /utf-8 /Fo:out\release\ /Fd:out\release\ /Fe:bin\release\mttlib /Iinclude /W4 /O2 /MD src\*
lib /OUT:bin\release\mttlib.lib out\release\*.obj
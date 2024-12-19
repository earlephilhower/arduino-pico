#include "Semihosting.h"
#include "SerialSemi.h"
#include "SemiFS.h"

SerialSemiClass SerialSemi;
FS SemiFS = FS(FSImplPtr(new semifs::SemiFSImpl()));

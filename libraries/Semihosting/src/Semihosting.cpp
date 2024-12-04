#include "Semihosting.h"

SerialSemiClass SerialSemi;
FS SemiFS = FS(FSImplPtr(new semifs::SemiFSImpl()));

#pragma once

namespace hwinfo {

    void init();
    void exit();

    namespace extra {
        const char* pid();
    }

    namespace opengl {
        const char* version();
    }
    
    namespace gpu {
        const char* vendor();
        const char* renderer();
        bool isIntel();
        bool isNVidia();
        bool isAMD();
        double usage();
        double usageMb();
        double availableMb();
        double physicalTotMb();
    }

    namespace cpu {
        int threadCount();
        const char* vendor();
        const char* brand();
        double usage(double cores[]);
    }

    namespace mem {
        double usageMb();
        double availableMb();
        double physicalTotMb();
    }

    namespace deps {
        const char* glfwVersion();
        const char* glewVersion();
        const char* imguiVersion();
    }

}

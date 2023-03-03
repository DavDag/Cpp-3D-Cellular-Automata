#pragma once

namespace hwinfo {

    void init();
    void exit();

    namespace opengl {
        const char* version();
    }
    
    namespace gpu {
        const char* vendor();
        const char* renderer();
        double usage();
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

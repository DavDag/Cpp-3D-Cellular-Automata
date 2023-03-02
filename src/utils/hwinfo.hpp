#pragma once

namespace runtimeinfo {

    namespace opengl {
        const char* version();
    }
    
    namespace gpu {
        const char* vendor();
        const char* renderer();
    }

    namespace cpu {
        int threadCount();
        const char* vendor();
        const char* brand();
    }

    namespace deps {
        const char* glfwVersion();
        const char* glewVersion();
        const char* imguiVersion();
    }

}

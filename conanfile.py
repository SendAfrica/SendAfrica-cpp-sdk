from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain
from conan.tools.files import copy, get
import os


class SendAfricaConan(ConanFile):
    name = "sendafrica"
    version = "1.0.1"
    license = "MIT"
    url = "https://github.com/SendAfrica/SendAfrica-cpp-sdk"
    homepage = "https://github.com/SendAfrica/SendAfrica-cpp-sdk"
    description = "Official C++ SDK for the SendAfrica SMS Infrastructure-as-a-Service API"
    topics = ("sms", "sendafrica", "africa", "tanzania")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "CMakeLists.txt", "cmake/*", "include/*", "src/*"
    requires = [
        "curl/[>=7.75.0]",
        "nlohmann-json/[>=3.10.0]",
        "openssl/[>=3.0.0]",
    ]

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["SENDAFRICA_BUILD_TESTS"] = False
        tc.variables["SENDAFRICA_BUILD_EXAMPLES"] = False
        tc.variables["SENDAFRICA_BUILD_SHARED"] = self.options.shared
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE*", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["sendafrica"]
        self.cpp_info.set_property("cmake_file_name", "sendafrica")
        self.cpp_info.set_property("cmake_target_name", "sendafrica::sendafrica")
        self.cpp_info.requires = [
            "curl::curl",
            "nlohmann-json::nlohmann-json",
            "openssl::crypto",
        ]

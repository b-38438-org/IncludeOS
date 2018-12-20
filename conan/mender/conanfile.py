#README to build botan 2.8.0 use conan create (botan/2.8.0@user/channel) path to this file
import shutil

from conans import ConanFile,tools,CMake

class MenderConan(ConanFile):
    settings= "os","arch","build_type","compiler"
    name = "mender"
    license = 'Apache-2.0'
    description = 'Run your application with zero overhead'
    generators = 'cmake'
    url = "http://www.includeos.org/"

    def requirements(self):
        self.requires("botan/2.8.0@includeos/test")
        self.requires("uzlib/v2.1.1@includeos/test")
        self.requires("liveupdate/{}@{}/{}".format(self.version,self.user,self.channel))
        #eventually
        #self.build_requires("includeos/%s@%s/%s"%(self.version,self.user,self.channel))
    def build_requirements(self):
        self.build_requires("rapidjson/1.1.0@{}/{}".format(self.user,self.channel))
        self.build_requires("GSL/2.0.0@{}/{}".format(self.user,self.channel))

    def source(self):
        repo = tools.Git(folder="includeos")
        repo.clone("https://github.com/hioa-cs/IncludeOS.git",branch="conan")

    def _arch(self):
        return {
            "x86":"i686",
            "x86_64":"x86_64"
        }.get(str(self.settings.arch))
    def _cmake_configure(self):
        cmake = CMake(self)
        cmake.definitions['ARCH']=self._arch()
        cmake.configure(source_folder=self.source_folder+"/includeos/lib/mender")
        return cmake

    def build(self):
        cmake = self._cmake_configure()
        cmake.build()

    def package(self):
        cmake = self._cmake_configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs=['mender']

    def deploy(self):
        self.copy("*.a",dst="lib",src="lib")
        self.copy("*",dst="include",src="include")

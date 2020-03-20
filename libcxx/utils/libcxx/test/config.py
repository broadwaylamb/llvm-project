#===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===----------------------------------------------------------------------===##

import os.path
import shutil
import sys

from libcxx.test.baseconfig import BaseConfiguration, intMacroValue

class Configuration(BaseConfiguration):

    def configure(self):
        super(Configuration, self).configure()
        self.configure_coroutines()
        self.configure_modules()
        self.configure_filesystem_compile_flags()
        self.configure_debug_mode()
    
    def configure_src_root(self):
        self.libcxx_src_root = self.get_lit_conf(
            'libcxx_src_root', os.path.dirname(self.config.test_source_root))
    
    def configure_compile_flags_exceptions(self):
        enable_exceptions = self.get_lit_bool('enable_exceptions', True)
        if not enable_exceptions:
            self.config.available_features.add('libcpp-no-exceptions')
            self.cxx.compile_flags += ['-fno-exceptions']
    
    def configure_compile_flags_rtti(self):
        enable_rtti = self.get_lit_bool('enable_rtti', True)
        if not enable_rtti:
            self.config.available_features.add('libcpp-no-rtti')
            self.cxx.compile_flags += ['-fno-rtti', '-D_LIBCPP_NO_RTTI']
    
    def configure_filesystem_compile_flags(self):
        static_env = os.path.join(self.libcxx_src_root, 'test', 'std',
                                  'input.output', 'filesystems', 'Inputs', 'static_test_env')
        static_env = os.path.realpath(static_env)
        assert os.path.isdir(static_env)
        self.cxx.compile_flags += ['-DLIBCXX_FILESYSTEM_STATIC_TEST_ROOT="%s"' % static_env]

        dynamic_env = os.path.join(self.config.test_exec_root,
                                   'filesystem', 'Output', 'dynamic_env')
        dynamic_env = os.path.realpath(dynamic_env)
        if not os.path.isdir(dynamic_env):
            os.makedirs(dynamic_env)
        self.cxx.compile_flags += ['-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT="%s"' % dynamic_env]
        self.exec_env['LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT'] = ("%s" % dynamic_env)

        dynamic_helper = os.path.join(self.libcxx_src_root, 'test', 'support',
                                      'filesystem_dynamic_test_helper.py')
        assert os.path.isfile(dynamic_helper)

        self.cxx.compile_flags += ['-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_HELPER="%s %s"'
                                   % (sys.executable, dynamic_helper)]
    
    def configure_compile_flags_header_includes(self):
        support_path = os.path.join(self.libcxx_src_root, 'test', 'support')
        self.configure_config_site_header()
        if self.cxx_stdlib_under_test != 'libstdc++' and \
           not self.target_info.is_windows():
            self.cxx.compile_flags += [
                '-include', os.path.join(support_path, 'nasty_macros.h')]
        if self.cxx_stdlib_under_test == 'msvc':
            self.cxx.compile_flags += [
                '-include', os.path.join(support_path,
                                         'msvc_stdlib_force_include.h')]
            pass
        if self.target_info.is_windows() and self.debug_build and \
                self.cxx_stdlib_under_test != 'msvc':
            self.cxx.compile_flags += [
                '-include', os.path.join(support_path,
                                         'set_windows_crt_report_mode.h')
            ]
        cxx_headers = self.get_lit_conf('cxx_headers')
        if cxx_headers == '' or (cxx_headers is None
                                 and self.cxx_stdlib_under_test != 'libc++'):
            self.lit_config.note('using the system cxx headers')
            return
        self.cxx.compile_flags += ['-nostdinc++']
        if cxx_headers is None:
            cxx_headers = os.path.join(self.libcxx_src_root, 'include')
        if not os.path.isdir(cxx_headers):
            self.lit_config.fatal("cxx_headers='%s' is not a directory."
                                  % cxx_headers)
        self.cxx.compile_flags += ['-I' + cxx_headers]
        if self.libcxx_obj_root is not None:
            cxxabi_headers = os.path.join(self.libcxx_obj_root, 'include',
                                          'c++build')
            if os.path.isdir(cxxabi_headers):
                self.cxx.compile_flags += ['-I' + cxxabi_headers]
    
    def configure_features(self):
        super(Configuration, self).configure_features()
        self.target_info.add_locale_features(self.config.available_features)
    
    def configure_debug_mode(self):
        debug_level = self.get_lit_conf('debug_level', None)
        if not debug_level:
            return
        if debug_level not in ['0', '1']:
            self.lit_config.fatal('Invalid value for debug_level "%s".'
                                  % debug_level)
        self.cxx.compile_flags += ['-D_LIBCPP_DEBUG=%s' % debug_level]
    
    def get_modules_enabled(self):
        return self.get_lit_bool('enable_modules',
                                default=False,
                                env_var='LIBCXX_ENABLE_MODULES')

    def configure_modules(self):
        modules_flags = ['-fmodules']
        if not self.target_info.is_darwin():
            modules_flags += ['-Xclang', '-fmodules-local-submodule-visibility']
        supports_modules = self.cxx.hasCompileFlag(modules_flags)
        enable_modules = self.get_modules_enabled()
        if enable_modules and not supports_modules:
            self.lit_config.fatal(
                '-fmodules is enabled but not supported by the compiler')
        if not supports_modules:
            return
        self.config.available_features.add('modules-support')
        module_cache = os.path.join(self.config.test_exec_root,
                                   'modules.cache')
        module_cache = os.path.realpath(module_cache)
        if os.path.isdir(module_cache):
            shutil.rmtree(module_cache)
        os.makedirs(module_cache)
        self.cxx.modules_flags += modules_flags + \
            ['-fmodules-cache-path=' + module_cache]
        if enable_modules:
            self.config.available_features.add('-fmodules')
            self.cxx.useModules()

    def configure_coroutines(self):
        if self.cxx.hasCompileFlag('-fcoroutines-ts'):
            macros = self._dump_macros_verbose(flags=['-fcoroutines-ts'])
            if '__cpp_coroutines' not in macros:
                self.lit_config.warning('-fcoroutines-ts is supported but '
                    '__cpp_coroutines is not defined')
            # Consider coroutines supported only when the feature test macro
            # reflects a recent value.
            if intMacroValue(macros['__cpp_coroutines']) >= 201703:
                self.config.available_features.add('fcoroutines-ts')

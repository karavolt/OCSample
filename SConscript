#******************************************************************
#
# Copyright: 2016, Samsung Electronics Co., Ltd.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Import('stacksamples_env')
ocsample_env = stacksamples_env.Clone()

target_os = stacksamples_env.get('TARGET_OS')
target_arch = stacksamples_env.get('TARGET_ARCH')

######################################################################
# Build flags
######################################################################
ocsample_env.PrependUnique(CPPPATH = [
    '#/resource/csdk/include',
    '#/resource/csdk/stack/include',
    '#/resource/csdk/security/include',
    '#/resource/oc_logger/include',
    '/usr/local/include',
])

compiler = ocsample_env.get('CXX')
if 'g++' in compiler:
    ocsample_env.AppendUnique(CXXFLAGS=['-std=c++0x', '-Wall'])

if target_os not in ['msys_nt', 'windows']:
    ocsample_env.PrependUnique(LIBS = [
        'connectivity_abstraction',
    ])

if target_arch in ['arm']:
    ocsample_env.PrependUnique(LIBS = [
        'wiringPi',
    ])

ocsample_env.PrependUnique(LIBS = [
    'octbstack',
])

if ocsample_env.get('SECURED') == '1':
	if ocsample_env.get('WITH_TCP') == True:
		ocsample_env.AppendUnique(LIBS = ['mbedtls', 'mbedx509','mbedcrypto'])

ocsample_env.AppendUnique(LIBPATH = [
    ocsample_env.get('BUILD_DIR'),
])

######################################################################
# Source files and Targets
######################################################################
occlient = ocsample_env.Program('occlient', ['occlient_none.cpp'])
ocserver = ocsample_env.Program('ocserver', ['ocserver_none.cpp'])

if target_arch in ['arm']:
    ocserver_led = ocsample_env.Program('ocserver_led', ['ocserver_led.cpp'])

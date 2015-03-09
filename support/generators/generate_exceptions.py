"""
  This script creates a _exceptions.inc and .h file from our exceptions.dat template.

  usage: generate_exceptions.py <file.inc> <file.h>
"""

import sys

def ucfirst(s) :
    if not s :
        return ""
    return s[0].upper() + s[1:]
def lcfirst(s) :
    if not s :
        return ""
    return s[0].lower() + s[1:]

#
# Read exceptions
#
exception_stack = []
cur_level = -1
exceptions = []
for line in open(sys.argv[1]):
    line = line.rstrip()
    if not line or line[0] == '#' :
        continue

    new_level = 0
    while line[new_level] == ' ' :
        new_level += 1
    new_level /= 4

    line = line.strip()

    if new_level == cur_level :
        exception_stack.pop()
    elif new_level < cur_level :
        while len(exception_stack) > new_level :
            exception_stack.pop()

    if len(exception_stack) > 0 :
        parent = exception_stack[len(exception_stack)-1]
    else :
        parent = ""

    exceptions.append((ucfirst(line), ucfirst(parent)))

    exception_stack.append(line)
    cur_level = new_level



#
# Write H file
#
fp = open(sys.argv[3], "w")

header = '''
/*
 Copyright (c) 2012-2015, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ___EXCEPTIONS_H__
#define ___EXCEPTIONS_H__


    /*
     * WARNING: THIS FILE IS AUTOGENERATED! PLEASE USE CREATE_EXCEPTIONS.PHP TO REGENERATE!
     */


'''
fp.write(header)

for exception in exceptions :
    (exception, parent) = exception

    fp.write("    // %s\n" % exception)
    fp.write("    t_exception_object Object_Exception_%s_struct;\n" % ucfirst(exception))
    fp.write("    #define Object_%s ((t_object *)&Object_Exception_%s_struct)\n" % (exception, exception))
    fp.write("\n")

footer = '''
#endif
'''
fp.write(footer)

fp.close()




#
# Write C file
#
fp = open(sys.argv[2], "w")

header = '''
/*
 Copyright (c) 2012-2015, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

    /*
     * WARNING: THIS FILE IS AUTOGENERATED! PLEASE USE CREATE_EXCEPTIONS.PHP TO REGENERATE!
     */

'''
fp.write(header)


fp.write("void object_exception_add_generated_exceptions(void) {\n")
for exception in exceptions :
    (exception, parent) = exception
    fp.write("    //%s\n" % exception)
    fp.write("    Object_Exception_%s_struct.attributes = Object_Exception_struct.attributes;\n" % exception)
    fp.write("    vm_populate_builtins(\"%s\", Object_%s);\n" % (lcfirst(exception), exception))
    fp.write("    \n")

fp.write("}\n")

fp.write("\n\n")


for exception in exceptions :
    (exception, parent) = exception

    if parent is not "" :
        parent += "_"
    fp.write("t_exception_object Object_Exception_%s_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS(\"%s\", objectTypeException, OBJECT_TYPE_CLASS | OBJECT_FLAG_IMMUTABLE, &exception_funcs, (t_object *)&Object_Exception_%sstruct, NULL, sizeof(t_exception_object_data)), { NULL, 0 } };\n" % (exception, ucfirst(exception), parent))

fp.close()

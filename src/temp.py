import os
import sys
import re

pattern = re.compile('vkCreate.+?\(.+?\)')
file = 'vulkan_initializer.cpp'
file = 'vulkan_helpers.cpp'

for i, line in enumerate(open(file)):
    for match in re.finditer(pattern, line):
        print(match.group())

import re
import requests
import subprocess

# Read the source file
with open(__import__('sys').argv[1], 'r') as file:
   lines = file.readlines()

# Find the URLs
url_pattern = re.compile(r'#include \"(http://.*?)\"')
urls = [url_pattern.search(line).group(1) for line in lines if url_pattern.search(line)]

# Download the files
for url in urls:
   response = requests.get(url)
   file_name = url.split('/')[-1] + '.h'
   with open(file_name, 'wb') as file:
       file.write(response.content)

for i, line in enumerate(lines):
   for url in urls:
       if url in line:
           file_name = url.split('/')[-1] + '.h'
           lines[i] = line.replace(f'\"{url}\"', f'\"{file_name}\"')

with open('source_file.mod.c', 'w') as file:
   file.writelines(lines)

# Compile the source file
subprocess.run(['gcc', '-o', 'output', 'source_file.mod.c', '-Wpedantic', '--pedantic'])

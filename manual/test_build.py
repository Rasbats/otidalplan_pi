#!/usr/bin/env python
# 

import base64
import os
import sys
import fnmatch
import shutil

import git
import pypandoc

replacements = {'{empty}[[/':'image::', '|':'[', ']]':']'}

def replace(input_dir, input_file):
    lines = []
    file_path = input_dir + "/" + input_file
    with open(file_path) as infile:
        for line in infile:
            for src, target in replacements.items():
                line = line.replace(src, target)
            lines.append(line)
    with open(file_path, 'w') as outfile:
        for line in lines:
            outfile.write(line)  

def convert(input_dir, output_format, output_dir):
    
    #data_files = os.listdir(input_dir)
    data_files = [ f for f in os.listdir(input_dir) if os.path.isfile(os.path.join(input_dir,f)) ]
    #os.chdir(output_dir)
    for filename in data_files:

      if filename != '.git':
        bit = os.path.splitext(filename)[0]
        pypandoc.convert_file(filename, to='asciidoc',  extra_args=['--extract-media=media'],
        outputfile= output_dir + "/" + str(bit) + '.adoc')
        #break
         
        print(filename)
      
    #output = pypandoc.convert_file(data_files[0], to='asciidoc', outputfile=output_dir + 'test_doc.adoc')
    #return output


def main():
    """Indeed: main function."""
   
    dir_path = os.path.dirname(os.path.realpath(__file__))   
    print("Using directory: " + dir_path)
    os.chdir('./src/docs/asciidoc')
    new_docs = os.getcwd()
    out_docs = os.getcwd()
    print("Current working directory: {0}".format(os.getcwd()))
    
    if not os.path.isdir("otidalplan_pi.wiki"):
       os.mkdir("otidalplan_pi.wiki")
    
    gitBit = "otidalplan_pi.wiki"
    
    new_len = len(fnmatch.filter(os.listdir("otidalplan_pi.wiki"), '*.*'))
    #print(new_len)  
    
    if new_len == 0:       
      my_git = "https://github.com/Rasbats/otidalplan_pi.wiki.git"
      git.Git(new_docs).clone(my_git)
      os.chdir(gitBit)
      gitBit = os.getcwd()
      files = [ f for f in os.listdir(gitBit) if os.path.isfile(os.path.join(gitBit,f)) ]
    
      for filename in files:
        #do some stuff
        print(filename)
      
      convert(gitBit, 'asciidoc', out_docs)
      os.chdir(new_docs)
        
    else: 
      os.chdir(gitBit)
      
      gitBit = os.getcwd()
      new_len = len(fnmatch.filter(os.listdir(gitBit), '*.md'))
      #print(new_len) 

      convert(gitBit, 'asciidoc', out_docs) 
      os.chdir(new_docs)  
  
  
    my_images = 'images/images'
    path = os.path.join(new_docs, my_images)  
    if not os.path.isdir(path):
        #print('start')      
        shutil.copytree(str(gitBit) + '/images', path) 
    else: 
        #print('here') 
        shutil.rmtree(path)      
        shutil.copytree(str(gitBit) + '/images', path) 
          
    replace(new_docs, 'Home.adoc')  
             
if __name__ == '__main__':
    main()



import os
import fnmatch
def get_source_files(dir, pattern) :
    source_files = []
    for root, dirs, files in os.walk(dir) :
        for file in files :
            if fnmatch.fnmatch(file, pattern) :
                source_files.append(os.path.join(root, file))
    return source_files
import os
import gzip
import shutil

# Path to your LittleFS data folder
web_dir = './data/www' 

def decompress_files(directory):
    print(f"Starting decompression in: {directory}")
    for root, dirs, files in os.walk(directory):
        for file in files:
            # Only look for .gz files
            if not file.endswith('.gz'):
                continue
                
            gz_path = os.path.join(root, file)
            # Remove the .gz extension for the new filename
            original_path = gz_path[:-3] 
            
            print(f"Decompressing: {file} -> {os.path.basename(original_path)}")
            with gzip.open(gz_path, 'rb') as f_in:
                with open(original_path, 'wb') as f_out:
                    shutil.copyfileobj(f_in, f_out)
            
            # Delete the .gz file to clean up
            os.remove(gz_path)

if __name__ == "__main__":
    if os.path.exists(web_dir):
        decompress_files(web_dir)
        print("\nDone! All files have been unzipped and .gz files removed.")
    else:
        print(f"Error: Could not find directory {web_dir}")
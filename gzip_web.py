import os
import gzip
import shutil

# This points to your specific folder path
web_dir = './data/www' 

def compress_folder(directory):
    print(f"Starting compression in: {directory}")
    for root, dirs, files in os.walk(directory):
        for file in files:
            # Don't double-compress already .gz files
            if file.endswith('.gz'):
                continue
            
            original_path = os.path.join(root, file)
            gz_path = original_path + '.gz'
            
            print(f"Compressing: {file}...")
            with open(original_path, 'rb') as f_in:
                with gzip.open(gz_path, 'wb', compresslevel=9) as f_out:
                    shutil.copyfileobj(f_in, f_out)
            
            # REMOVE original to save LittleFS space and stop the 'not found' logs
            os.remove(original_path)

if __name__ == "__main__":
    if os.path.exists(web_dir):
        compress_folder(web_dir)
        print("\nSuccess! All files compressed to .gz and originals removed.")
    else:
        print(f"Error: Could not find directory {web_dir}")
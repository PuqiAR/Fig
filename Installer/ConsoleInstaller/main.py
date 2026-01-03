# Fig online installer - Python

# Copyright (C) 2020-2026 PuqiAR

from tqdm import tqdm
import requests
import json

import zipfile
import platform
from os import path as ospath
from os import mkdir,chdir

from sys import exit, argv

VERSION = '1.0.0'
GITEA_URL = 'https://git.fig-lang.cn'

Windows = 0
Linux = 2
Darwin = 3

def getOSName() -> int|str:
    name = platform.system()
    if name == 'Windows':
        return Windows
    elif name == 'Linux':
        return Linux
    elif name == 'Darwin':
        return Darwin
    else:
        return name

def resolveInstallPath(os: int):
    default_path = {
        Windows : r'C:\Program Files\Fig',
        Linux : r'/opt/Fig'
    }
    if os not in default_path:
        raise
    
    return default_path[os]

def getReleases():
    api_postfix = '/api/v1/repos/PuqiAR/Fig/releases'
    api_url = GITEA_URL + api_postfix
    
    rel = requests.get(api_url).text
    # print(rel)
    return rel

def install(url, path:str) -> None:
    if not ospath.exists(path):
        mkdir(path)
    
    filename = path.split('/')[-1]
    
    response = requests.get(url, stream=True)
    total_size = int(response.headers.get('content-length', 0))
    
    with tqdm(total=total_size, unit='B', unit_scale=True, desc=filename) as b:
        with open(filename, 'wb') as f:
            for data in response.iter_content(chunk_size=1024):
                f.write(data)
                b.update(len(data))
    
    print(f'{filename} download completed.')
    print(f'unziping to {path} ...')
    with zipfile.ZipFile(filename) as zip:
        zip.extractall(path)
    
    print('unziping completed')
    

def main() -> None:
    print(f'== Fig Online Installer v{VERSION} ==')
    
    osName = getOSName()
    if osName is int or osName == 'Darwin':
        print(f'Unsupport os: {osName}')
        exit(1)
    
    dpath = resolveInstallPath(osName)
    print(f'Install to (default: {dpath}): ', end='')

    path = input()
    if path.isspace():
        path = dpath
    print()
        
    releases = json.loads(getReleases())
    # print(type(releases))
    if not isinstance(releases, list):
        print('get releases failed!')
        exit(1)
    if len(releases) == 0:
        print('No version has been released!')
        exit(0)
    
    print('There are {} versions:' % len(releases))
    i = 1
    for release in releases:
        print(f"    {i} {release['name']} - {release['body']}")
        print(f"        @{release['published_at']}")
        
        if i >= 3:
            print(f'    .....')
            break # 展示前3个
    
    insVersion: dict = releases[0]
    print(f"Which version do you want to install({insVersion['name']} if empty)(e.g. x.x.x/index)? ", end='')
    usrInput = input()
    
    print()
    
    version = insVersion
    if not usrInput.isspace():
  
        if '.' in usrInput:
            for release in releases:
                if release['tag_name'].find(usrInput) != -1:
                    version = release
        else:
            try:
                index = int(usrInput)
                if index > 3 or index < 1 or (index > len(releases)):
                    print(f'Invalid index {usrInput} to install')
                    exit(1)
                version = releases[index - 1]
            except Exception:
                print('Invalid input')
                exit(1)
    
    print('\n================================')
    print(f"Installing Fig-{version['tag_name']}")
    
    url = None
    for asset in version['assets']:
        if asset['name'].find(osName) != -1:
            url = asset['browser_download_url']
            break
    
    if url:
        install(url, path)
    else:
        print('Could not find artifact:')
        print(version['assets'])
        exit(1)
    
    print(f"Fig-{version['tag_name']} install successfully to {path} !")
    exit(0)
        
if __name__ == '__main__':
    path = ospath.realpath(ospath.dirname(argv[0]))
    chdir(path)
    main()
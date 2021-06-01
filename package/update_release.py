#!/usr/bin/env python3

from github import Github
import argparse

parser = argparse.ArgumentParser(description='Refresh a set of files in an existing release')
parser.add_argument('--token', help="Github Personal Access Token (PAT)", type=str, required=True)
parser.add_argument('--repo', help="Repository", type=str, required=True)
parser.add_argument('--tag', help="Release tag", type=str, required=True)
parser.add_argument('files', nargs=argparse.REMAINDER)
args = parser.parse_args()

if len(args.files) == 0:
    print("ERROR:  No files specified")
    quit()

gh = Github(login_or_token=args.token)
repo = gh.get_repo(str(args.repo))
for fn in args.files:
    release = repo.get_release(args.tag)
    for asset in release.get_assets():
        if asset.name == fn:
            print("Found '" + fn + "', updating")
            asset.delete_asset()
            release.upload_asset(fn)

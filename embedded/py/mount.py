
"""
Badgerloop - File System Mounting
"""

# figure out permission problems

def run(args):
    """ """

    print "mount"
    return 0

def init_args(parser):
    """ """

    subp = parser.add_parser("mount")
    subp.set_defaults(handler=run)

    return 0
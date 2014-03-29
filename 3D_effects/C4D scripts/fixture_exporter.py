#   Fixtures Exporter
# 
#   Created by Andrea Cuius
#   andrea@nocte.co.uk
#   nocte.co.uk
# 

import c4d

def main():
    
    obj         = op.GetObject()                                                                # root object
    
    # open file
    filename    = str( c4d.storage.GeGetC4DPath( c4d.C4D_PATH_DESKTOP ) ) + "/" + obj.GetName() + ".csv"
    file        = open( filename, 'w' )
                    
    # write coords
    for child in obj.GetChildren():
        mat         = child.GetMg()                                                             # obj global matrix
        objPos      = child.GetMg().off / 100.0                                                 # convert position to meters
        file.write( str( objPos.x ) + "," + str( objPos.y ) + "," + str( objPos.z ) + "\n" )    # write position
        
    file.close()
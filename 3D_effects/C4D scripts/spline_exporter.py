#   Spline Exporter
# 
#   Created by Andrea Cuius
#   andrea@nocte.co.uk
#   nocte.co.uk
# 

import c4d

def main():
    print "Export Splines"
    
    obj     = op.GetObject()
    splines = obj.GetChildren()
    
    if not splines:
        print "no objects!"
    
    for s in splines:
        filename    = str( c4d.storage.GeGetC4DPath( c4d.C4D_PATH_DESKTOP ) ) + "/" + s.GetName() + ".txt"
        file        = open( filename, 'w' )
        mat         = s.GetMg()
        
        for p in s.GetAllPoints():
            pos = mat.Mul(p) * 0.01                                                         #export in m
            file.write( str( pos.x ) + "," + str( pos.y ) + "," + str( pos.z ) + "\n" )
                       
        file.close()
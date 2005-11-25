
proc main() {
    print(nl())
    forfam(f,c) {
	print(d(c),": ",key(f),nl()) 
	print("    children\n")
	children(f,i,c2) {
	    print("    ",d(c2),": ",key(i),nl()) 
	}
	print("    spouses\n")
	spouses(f,s,c2) {
	    print("    ",d(c2),": ",key(s),nl()) 
	}
    }
}

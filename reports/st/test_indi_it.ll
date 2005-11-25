proc main() {
    print(nl())
    forindi(i,c) { 
	print(d(c),": ",key(i),nl())
	print("    Parents\n")
	Parents(i,f,c2) {
	    print("    ",d(c2),": ",key(f),nl())
	}
	print("    families\n")
	families(i,f,s,c2) {
	    print("    ",d(c2),": ",key(f)," ",key(s),nl())
	}
	print("    spouses\n")
	spouses(i,s,f,c2) {
	    print("    ",d(c2),": ",key(s)," ",key(f),nl())
	}
	print("    mothers\n")
	mothers(i,m,f,c2) {
	    print("    ",d(c2),": ",key(m)," ",key(f),nl())
	}
	print("    fathers\n")
	fathers(i,fa,f,c2) {
	    print("    ",d(c2),": ",key(fa)," ",key(f),nl())
	}
    }
}

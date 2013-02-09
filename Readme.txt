L�sningen inneholder et program for � laste inn og vise 3d modeller med OpenGL, i SDL. 

Man kan rotere modellene ved � trykke en museknapp og dra musen. Ved musetrykk lagrer man en vector3d posisjon p� en virtuell sphere, og n�r en mousemotion skjer (musen beveger seg) f�r man tak i en ny vector3d posisjon p� spheren. Ved � ta kryssproduktet mellom disse to vectorene f�r vi aksen man skal rotere rundt, og asin av dot produktet av de to vectorene gir rotasjons gradene man skal rotere rundt denne aksen.

Aksen og gradene brukes til � konstruere queternionen som blir brukt til � lage en 4x4 matrise, som blir ganget inn med view matrisen.

Zoom er implementert ved � endre p� FoV verdien n�r man trykker page up/down, eller scroller med musen. N�r FoV verdien �kes blir "linsen" til kameraet bredere, og modellen blir mindre p� skjermen. N�r FoV verdien minskes, blir "linsen" smalere, og modellen blir st�rre p� skjermen. Jeg har valgt � bruke �n float som FoV, og gj�re denne st�rre/mindre direkte, istedenfor � ha en konstant FoV som deles p� en zoomFactor. Dette da det f�ltes mer naturlig visuelt. Jeg har lagt til en max limit(179.999f) og min limit (0.001f) for FoV verdien.

N�r modellen lastes inn, loopes det igjennom alle verticene, og de st�reste og minste x/y/z verdiene blir funnet for � kunne definere en "bounding box". Lengden mellom max og min xyz brukes for � skalere modellen slik at den passer inn i den virtuelle spheren som brukes til � rotere modellen (1/distance).


-forklare hyperbolic sheet
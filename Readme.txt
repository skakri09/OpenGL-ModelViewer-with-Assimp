Løsningen inneholder et program for å laste inn og vise 3d modeller med OpenGL, i SDL. 

Man kan rotere modellene ved å trykke en museknapp og dra musen. Ved musetrykk lagrer man en vector3d posisjon på en virtuell sphere, og når en mousemotion skjer (musen beveger seg) får man tak i en ny vector3d posisjon på spheren. Ved å ta kryssproduktet mellom disse to vectorene får vi aksen man skal rotere rundt, og asin av dot produktet av de to vectorene gir rotasjons gradene man skal rotere rundt denne aksen.

Aksen og gradene brukes til å konstruere queternionen som blir brukt til å lage en 4x4 matrise, som blir ganget inn med view matrisen.

Zoom er implementert ved å endre på FoV verdien når man trykker page up/down, eller scroller med musen. Når FoV verdien økes blir "linsen" til kameraet bredere, og modellen blir mindre på skjermen. Når FoV verdien minskes, blir "linsen" smalere, og modellen blir større på skjermen. Jeg har valgt å bruke én float som FoV, og gjøre denne større/mindre direkte, istedenfor å ha en konstant FoV som deles på en zoomFactor. Dette da det føltes mer naturlig visuelt. Jeg har lagt til en max limit(179.999f) og min limit (0.001f) for FoV verdien.

Når modellen lastes inn, loopes det igjennom alle verticene, og de støreste og minste x/y/z verdiene blir funnet for å kunne definere en "bounding box". Lengden mellom max og min xyz brukes for å skalere modellen slik at den passer inn i den virtuelle spheren som brukes til å rotere modellen (1/distance).


-forklare hyperbolic sheet

Features i "modelvieweren":
-Lasting av modellformater assimp støtter. Det er lagt med eksempel modeller i .obj og .3ds format.
-Rotering av innlastede modeller ved bruk av en virtual trackball og quaternioner.
-Forbedret trackball funksjonalitet grunnet implementasjon av hyperbolic sheet.
-Shading av innlastede modeller. Følgende shader modeller er implementert:
	Phong shading 
	Flat Shading	
	Wireframe	
	Hidden Line	
	Diffuse Textured
-Interleaved VBO rendering
-Skikkelig indexert rendering med glDrawElements
-Innlasting av modeller og directory browsing i realtime ved bruk av kommandoer i consoll vinduet.
-Alle andre Mandatory punkter for innleveringen.

I model klassen har jeg lagt til funksjonalitet for å laste normaler, teksturkoordinater og leting etter eventuelle materialer. Jeg regner ut max og min dimensjonene på modellen for å kunne skalere den til å passe inn i trackball spheren. Istedenfor å bruke indexene assimp gir oss til å laste inn vertex data flere ganger, laster jeg inn en mesh sin vertex data for seg, og indexer for seg, og bruker isteden indexene til å rendre modellen med indexert rendering (glDrawElements). For at jeg skal kunne holde flere meshes i en interleaved VBO, passer jeg på å offsette indexene riktig.

Eventuelle materialer som blir funnet under model innlastingen blir lastet inn med DevIL i en ny klasse, TextureFactory. TextureFactory klassen er en singleton som laster inn texturer, putter dem i grafikk minnet, og lagrer texturIDene i et map. Når jeg så rendrer modellen, henter jeg ut IDene fra textureFactoriet. 


Trackball klassen er fullført med bruk av quaternioner slik vi har lært i forelesningene, men istedenfor å låse rotasjoner til innsiten av den virtuelle spheren, har jeg implementert hyperbolic sheet funksjonalitet som lar deg rotere fritt hvorenn du drar musen fra på skjermen.


I gamemanager klassen er det gjort en del endringer for å la deg bytte mellom shader programmer realtime. Jeg valgte å lage en shader for hver rendermode (phong, flat, wireframe, hiddenline og textured) selv om det er mye likt i de forskjellige shaderne, da det virker mer oversiktelig å ha en shader for hver rendermode. Det ble aldri til at jeg fikk laget et pent system for å håndtere binding av uniforms og attributes, så det er en del if-tester for å holde styr på hva som blir bundet når, basert på hvilke shader som er aktiv. Dette kunne vært gjort mer galant.

Modellen som blir lastet inn når man kjører programmet kan spesifiseres som commandline argument.

Til slutt har jeg også laget et lite system for å kunne browse subdirectories til models mappen, skrive ut innholdet og å laste inn modeller fra console vinduet. Dette systemet bruker boost::filesystem. Det er en veldig enkel syntax for å gjøre dette, den er som følger:

Press enter i model-vieweren for å entre "console mode".
"cd" hopper en directory mappe ut. Man kan ikke gå lengre ut enn models mappen.
"cd " etterfulgt av et valid sub-directory navn hopper inn i det directoriet. F.eks: "cd nascar".
"dir" skriver ut innholdet av en directory
"load " etterfulgt av et valid modell-filnavn i current direcotry laster inn modellen. F.eks: "load bunny.obj".
Når man er i "console mode" kan man ikke bruke modelvieweren. For å komme ut av "console mode" og tilbake til å bruke modelvieweren kan man enten successfully laste inn en modell, eller skrive "exit". 

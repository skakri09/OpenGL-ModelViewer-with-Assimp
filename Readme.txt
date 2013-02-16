
Features i "modelvieweren":
-Lasting av modellformater assimp st�tter. Det er lagt med eksempel modeller i .obj og .3ds format.
-Rotering av innlastede modeller ved bruk av en virtual trackball og quaternioner.
-Forbedret trackball funksjonalitet grunnet implementasjon av hyperbolic sheet.
-Shading av innlastede modeller. F�lgende shader modeller er implementert:
	Phong shading 
	Flat Shading	
	Wireframe	
	Hidden Line	
	Diffuse Textured
-Interleaved VBO rendering
-Skikkelig indexert rendering med glDrawElements
-Innlasting av modeller og directory browsing i realtime ved bruk av kommandoer i consoll vinduet.
-Alle andre Mandatory punkter for innleveringen.

I model klassen har jeg lagt til funksjonalitet for � laste normaler, teksturkoordinater og leting etter eventuelle materialer. Jeg regner ut max og min dimensjonene p� modellen for � kunne skalere den til � passe inn i trackball spheren. Istedenfor � bruke indexene assimp gir oss til � laste inn vertex data flere ganger, laster jeg inn en mesh sin vertex data for seg, og indexer for seg, og bruker isteden indexene til � rendre modellen med indexert rendering (glDrawElements). For at jeg skal kunne holde flere meshes i en interleaved VBO, passer jeg p� � offsette indexene riktig.

Eventuelle materialer som blir funnet under model innlastingen blir lastet inn med DevIL i en ny klasse, TextureFactory. TextureFactory klassen er en singleton som laster inn texturer, putter dem i grafikk minnet, og lagrer texturIDene i et map. N�r jeg s� rendrer modellen, henter jeg ut IDene fra textureFactoriet. 


Trackball klassen er fullf�rt med bruk av quaternioner slik vi har l�rt i forelesningene, men istedenfor � l�se rotasjoner til innsiten av den virtuelle spheren, har jeg implementert hyperbolic sheet funksjonalitet som lar deg rotere fritt hvorenn du drar musen fra p� skjermen.


I gamemanager klassen er det gjort en del endringer for � la deg bytte mellom shader programmer realtime. Jeg valgte � lage en shader for hver rendermode (phong, flat, wireframe, hiddenline og textured) selv om det er mye likt i de forskjellige shaderne, da det virker mer oversiktelig � ha en shader for hver rendermode. Det ble aldri til at jeg fikk laget et pent system for � h�ndtere binding av uniforms og attributes, s� det er en del if-tester for � holde styr p� hva som blir bundet n�r, basert p� hvilke shader som er aktiv. Dette kunne v�rt gjort mer galant.

Modellen som blir lastet inn n�r man kj�rer programmet kan spesifiseres som commandline argument.

Til slutt har jeg ogs� laget et lite system for � kunne browse subdirectories til models mappen, skrive ut innholdet og � laste inn modeller fra console vinduet. Dette systemet bruker boost::filesystem. Det er en veldig enkel syntax for � gj�re dette, den er som f�lger:

Press enter i model-vieweren for � entre "console mode".
"cd" hopper en directory mappe ut. Man kan ikke g� lengre ut enn models mappen.
"cd " etterfulgt av et valid sub-directory navn hopper inn i det directoriet. F.eks: "cd nascar".
"dir" skriver ut innholdet av en directory
"load " etterfulgt av et valid modell-filnavn i current direcotry laster inn modellen. F.eks: "load bunny.obj".
N�r man er i "console mode" kan man ikke bruke modelvieweren. For � komme ut av "console mode" og tilbake til � bruke modelvieweren kan man enten successfully laste inn en modell, eller skrive "exit". 

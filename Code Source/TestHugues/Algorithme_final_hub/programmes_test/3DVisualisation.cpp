#include "3DVisualisation.hpp"

vector<vector<V3>> buildVectorsListFor3DEnvironment(vector<V3> pPoints)
{
    vector<vector<V3>> vResult;
    int j = 1;
    int vNbPoints = pPoints.size();
    vector<V3> vCurrentVector;

    for (int i = 0; i < vNbPoints; i++)
    {
        vCurrentVector.push_back(pPoints[i]);
        vCurrentVector.push_back(pPoints[j]);
        vResult.push_back(vCurrentVector);
        vCurrentVector.clear();
        j = (j+1) % vNbPoints;
    }

    return vResult;
}

vector<vector<V3>> concatenateVectorsList(vector<vector<V3>> pList1, vector<vector<V3>> pList2)
{
    vector<vector<V3>> vFinalVectorList;
    vFinalVectorList.reserve(pList1.size() + pList2.size());
    vFinalVectorList.insert(vFinalVectorList.end(), pList1.begin(), pList1.end());
    vFinalVectorList.insert(vFinalVectorList.end(), pList2.begin(), pList2.end());

    return vFinalVectorList;
}

vector<vector<V3>> convertVectorsListAxisToRayLib(vector<vector<V3>> pVectorsList)
{
    vector<vector<V3>> vResult;

    for (int i = 0; i < pVectorsList.size(); i++)
    {
        vResult.push_back(convertPointsListAxisToRayLib(pVectorsList[i]));
    }

    return vResult;
}

vector<V3> concatenatePointsList(vector<V3> pList1, vector<V3> pList2)
{
    vector<V3> vFinalVectorList;
    vFinalVectorList.reserve(pList1.size() + pList2.size());
    vFinalVectorList.insert(vFinalVectorList.end(), pList1.begin(), pList1.end());
    vFinalVectorList.insert(vFinalVectorList.end(), pList2.begin(), pList2.end());

    return vFinalVectorList;
}

vector<V3> convertPointsListAxisToRayLib(vector<V3> pPointsList)
{
    vector<V3> vResult;

    for (int i = 0; i < pPointsList.size(); i++)
    {
        vResult.push_back(convertPointAxisToRayLib(pPointsList[i]));
    }

    return vResult;
}

V3 convertPointAxisToRayLib(V3 pPoint)
{
    return V3(pPoint.getX(), pPoint.getZ(), -pPoint.getY());
}

void view3DEnvironment(vector<V3> pPoints, vector<vector<V3>> pVectors)
{
    const int screenWidth = 1000;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Visualisation Interactive 3D - Mes Vecteurs");
    
    // Configuration initiale de la caméra
    Camera3D camera = { 0 };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Ce que la caméra regarde (l'origine)
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // L'axe vertical reste l'axe Y
    camera.fovy = 45.0f;                                // Champ de vision
    camera.projection = CAMERA_PERSPECTIVE;

    // Paramètres de contrôle de la caméra (Coordonnées sphériques)
    float radius = 15.0f;       // Distance de la caméra par rapport au centre (gère le Zoom)
    float alpha = 0.78f;        // Angle de rotation horizontal (gauche/droite) -> ~45° au départ
    float beta = 0.6f;          // Angle de rotation vertical (haut/bas) -> ~35° au départ

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        // 1. GESTION DU ZOOM (Molette de la souris)
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            radius -= wheel * 1.0f; // On ajuste la sensibilité du zoom ici (multiplier par 1.0)
            
            // On définit des limites pour éviter d'aller trop près ou trop loin
            if (radius < 2.0f) radius = 2.0f;
            if (radius > 50.0f) radius = 50.0f;
        }

        // 2. GESTION DE LA ROTATION (Clic gauche enfoncé + glisser la souris)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // GetMouseDelta() donne le déplacement de la souris depuis la dernière image
            Vector2 mouseDelta = GetMouseDelta();
            
            // On modifie les angles en fonction du déplacement de la souris
            // Le multiplicateur 0.005f sert à régler la sensibilité de la rotation
            alpha -= mouseDelta.x * 0.005f;
            beta  += mouseDelta.y * 0.005f;

            // On limite la rotation verticale à un peu moins de 90° (1.5 radian) 
            // pour éviter que la caméra ne se retourne complètement au pôle Nord/Sud
            if (beta > 1.5f) beta = 1.5f;
            if (beta < -1.5f) beta = -1.5f;
        }

        // 3. CALCUL DE LA NOUVELLE POSITION DE LA CAMÉRA
        // On convertit les angles (alpha, beta) et la distance (radius) en coordonnées de l'espace (X, Y, Z)
        camera.position.x = camera.target.x + radius * cosf(beta) * sinf(alpha);
        camera.position.y = camera.target.y + radius * sinf(beta);
        camera.position.z = camera.target.z + radius * cosf(beta) * cosf(alpha);

        // 4. RENDU GRAPHIQUE
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);

                // Une grille au sol pour se repérer dans l'espace
                DrawGrid(20, 1.0f);

                // --- TES DONNÉES 3D ---
                // on affiche tous nos points en Rouge
                V3 vCurrentPoint;

                for (int i = 0; i < pPoints.size(); i++)
                {
                    vCurrentPoint = pPoints[i];
                    DrawSphere((Vector3){vCurrentPoint.getX(), vCurrentPoint.getY(), vCurrentPoint.getZ()}, 0.1f, RED);
                }

                
                // on affiche tous les vecteurs en Bleu
                vector<V3> vCurrentVector;
                V3 vCurrentPointA;
                V3 vCurrentPointB;
                Color vCurrentVectorColor;

                for (int i = 0; i < pVectors.size(); i++)
                {
                    vCurrentVector = pVectors[i];
                    vCurrentPointA = vCurrentVector[0];
                    vCurrentPointB = vCurrentVector[1];
                    DrawLine3D((Vector3){vCurrentPointA.getX(), vCurrentPointA.getY(), vCurrentPointA.getZ()},
                                 (Vector3){vCurrentPointB.getX(), vCurrentPointB.getY(), vCurrentPointB.getZ()}, BLUE);
                }

            EndMode3D();

            // Instructions textuelles affichées discrètement en haut à gauche
            DrawText("Clic Gauche + Glisser : Tourner la vue", 10, 10, 20, DARKGRAY);
            DrawText("Molette : Zoomer / Dezoomer", 10, 35, 20, DARKGRAY);
            
        EndDrawing();
    }

    CloseWindow();
}
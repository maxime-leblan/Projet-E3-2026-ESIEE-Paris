#include "LissagePlanExclusion.hpp"
#include <iostream>

namespace LissageVehicule {

    // --- FONCTION 1 : LE PLAN MOYEN (Régression Orthogonale via ACP) ---
    PlanLocal calculerPlanMoyen(const std::vector<V3>& points) {
        PlanLocal plan;
        int n = points.size();

        if (n < 3) {
            // Sécurité : Impossible de faire un plan avec moins de 3 points
            plan.centre = V3(0,0,0);
            plan.normale = V3(0,0,1);
            plan.axeU = V3(1,0,0);
            plan.axeV = V3(0,1,0);
            return plan;
        }

        // 1. Calcul du centre de gravité (Moyenne)
        Eigen::Vector3f centre(0, 0, 0);
        for (const auto& p : points) {
            centre.x() += p.getX();
            centre.y() += p.getY();
            centre.z() += p.getZ();
        }
        centre /= n;

        // 2. Création de la matrice de covariance
        Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();
        for (const auto& p : points) {
            Eigen::Vector3f pt(p.getX() - centre.x(), p.getY() - centre.y(), p.getZ() - centre.z());
            covariance += pt * pt.transpose();
        }
        covariance /= (n - 1);

        // 3. Résolution des vecteurs propres (EigenSolver)
        // La normale du plan correspond à la direction de plus faible variance (plus petite valeur propre)
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);
        Eigen::Vector3f normale_eigen = solver.eigenvectors().col(0).normalized();

        // 4. Création du repère 2D local (Axe U et Axe V) sur le plan
        // On choisit un vecteur arbitraire qui n'est pas parallèle à la normale pour initier le repère
        Eigen::Vector3f arbitraire(1, 0, 0);
        if (std::abs(normale_eigen.dot(arbitraire)) > 0.9f) { 
            arbitraire = Eigen::Vector3f(0, 1, 0); // On change si la normale est trop proche de X
        }
        
        Eigen::Vector3f axe_u = arbitraire.cross(normale_eigen).normalized();
        Eigen::Vector3f axe_v = normale_eigen.cross(axe_u).normalized();

        // 5. Remplissage de la structure finale
        plan.centre = V3(centre.x(), centre.y(), centre.z());
        plan.normale = V3(normale_eigen.x(), normale_eigen.y(), normale_eigen.z());
        plan.axeU = V3(axe_u.x(), axe_u.y(), axe_u.z());
        plan.axeV = V3(axe_v.x(), axe_v.y(), axe_v.z());

        // Calcul de l'équation cartésienne ax + by + cz + d = 0
        plan.a = plan.normale.getX();
        plan.b = plan.normale.getY();
        plan.c = plan.normale.getZ();
        plan.d = -(plan.a * plan.centre.getX() + plan.b * plan.centre.getY() + plan.c * plan.centre.getZ());

        return plan;
    }


    // --- FONCTION 2 : PROJECTION ET RÉÉCHANTILLONNAGE 64 POINTS ---
    std::vector<Point2D> echantillonner64Points(const PlanLocal& plan, const std::vector<V3>& points3D) {
        std::vector<Point2D> pointsProjetes;
        std::vector<Point2D> pointsFinaux;

        if (points3D.empty()) return pointsFinaux;

        // 1. PROJECTION SUR LE NOUVEAU PLAN 2D
        // On utilise le produit scalaire pour trouver les coordonnées X et Y locales
        Eigen::Vector3f c(plan.centre.getX(), plan.centre.getY(), plan.centre.getZ());
        Eigen::Vector3f u(plan.axeU.getX(), plan.axeU.getY(), plan.axeU.getZ());
        Eigen::Vector3f v(plan.axeV.getX(), plan.axeV.getY(), plan.axeV.getZ());

        for (const auto& p : points3D) {
            Eigen::Vector3f pt(p.getX() - c.x(), p.getY() - c.y(), p.getZ() - c.z());
            pointsProjetes.push_back({ pt.dot(u), pt.dot(v) });
        }

        // On ferme la boucle artificiellement pour le calcul du périmètre
        pointsProjetes.push_back(pointsProjetes[0]);

        // 2. CALCUL DES DISTANCES ET DU PÉRIMÈTRE
        std::vector<float> distancesCumulees;
        distancesCumulees.push_back(0.0f);
        
        float perimetreTotal = 0.0f;
        for (size_t i = 0; i < pointsProjetes.size() - 1; ++i) {
            float dx = pointsProjetes[i+1].x - pointsProjetes[i].x;
            float dy = pointsProjetes[i+1].y - pointsProjetes[i].y;
            float distanceSegment = std::sqrt(dx*dx + dy*dy);
            
            perimetreTotal += distanceSegment;
            distancesCumulees.push_back(perimetreTotal);
        }

        // 3. RÉÉCHANTILLONNAGE (Interpolation linéaire)
        float longueurStep = perimetreTotal / 63.0f; // On divise par 63 pour avoir 64 points (0 à 63)
        pointsFinaux.push_back(pointsProjetes[0]);   // Le point 0 est toujours le point de départ

        int indexSegmentActuel = 0;

        // On cherche les 62 points du milieu
        for (int i = 1; i < 63; ++i) {
            float distanceCible = i * longueurStep;

            // On avance dans les segments jusqu'à trouver celui qui contient la distance cible
            while (indexSegmentActuel < pointsProjetes.size() - 1 && 
                   distancesCumulees[indexSegmentActuel + 1] < distanceCible) {
                indexSegmentActuel++;
            }

            // Interpolation mathématique (Théorème de Thalès)
            float distanceDansLeSegment = distanceCible - distancesCumulees[indexSegmentActuel];
            float longueurDuSegment = distancesCumulees[indexSegmentActuel + 1] - distancesCumulees[indexSegmentActuel];
            
            float ratio = (longueurDuSegment > 0.0001f) ? (distanceDansLeSegment / longueurDuSegment) : 0.0f;

            Point2D ptInterpole;
            ptInterpole.x = pointsProjetes[indexSegmentActuel].x + ratio * (pointsProjetes[indexSegmentActuel+1].x - pointsProjetes[indexSegmentActuel].x);
            ptInterpole.y = pointsProjetes[indexSegmentActuel].y + ratio * (pointsProjetes[indexSegmentActuel+1].y - pointsProjetes[indexSegmentActuel].y);
            
            pointsFinaux.push_back(ptInterpole);
        }

        // Le 64ème point (index 63) DOIT être exactement le même que le 1er point
        pointsFinaux.push_back(pointsProjetes[0]);

        return pointsFinaux;
    }
}


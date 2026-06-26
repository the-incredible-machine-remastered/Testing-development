// ============================================================================
// COPIA DE REFERENCIA: cortar_cuerdas_con_tijeras()
// Extraido de: src/fisica/motor_fisica.h
// Fecha: 2026-06-25
// ============================================================================

void cortar_cuerdas_con_tijeras() {
    std::vector<int> ids_a_eliminar;
    std::vector<EntidadFisica*> entidades_nuevas;

    for (auto* e : entidades) {
        auto* tijera = dynamic_cast<Tijera*>(e);
        if (!tijera || !tijera->get_fue_activada() || tijera->get_ya_corto_cuerdas()) continue;

        Vector2D t_min = tijera->get_min() - Vector2D(5, 5);
        Vector2D t_max = tijera->get_max() + Vector2D(5, 5);

        for (auto* e2 : entidades) {
            auto* cuerda = dynamic_cast<Cuerda*>(e2);
            if (!cuerda) continue;

            // puntos: [extremo_a(0), sop0(1), sop1(2), ..., extremo_b(n-1)]
            std::vector<Vector2D> puntos;
            if (!cuerda->obtener_puntos(entidades, puntos)) continue;

            int seg_cortado = -1;
            for (size_t k = 1; k < puntos.size() && seg_cortado < 0; ++k) {
                if (segmento_cruza_aabb(puntos[k-1], puntos[k], t_min, t_max))
                    seg_cortado = static_cast<int>(k);
            }
            if (seg_cortado < 0) continue;

            ids_a_eliminar.push_back(cuerda->get_id());

            const std::vector<int>& sops = cuerda->get_soportes_id();
            int n = static_cast<int>(puntos.size());
            int num_sops = static_cast<int>(sops.size());

            // sops[i] corresponde a puntos[i+1]
            // seg_cortado: indice del punto FINAL del segmento cortado
            int num_sops_a = seg_cortado - 1; // soportes antes del corte
            int num_sops_b = num_sops - num_sops_a; // soportes despues del corte

            // LADO A: termina en penultimo soporte (sops[num_sops_a-2])
            // El ultimo soporte del lado A (sops[num_sops_a-1]) se descarta
            // Bolita suelta cuelga del penultimo soporte
            if (num_sops_a >= 2) {
                int id_b_a = sops[num_sops_a - 2]; // penultimo soporte = torque_izq
                std::vector<int> sops_a(sops.begin(), sops.begin() + num_sops_a - 2);
                double lon_a = 0.0;
                for (int i = 0; i < seg_cortado - 2; ++i)
                    lon_a += Vector2D::distancia(puntos[i], puntos[i+1]);
                if (lon_a > MathUtils::EPSILON) {
                    AnclajeCuerda anc_b_a{ id_b_a, TipoAnclajeCuerda::SoporteFijo };
                    entidades_nuevas.push_back(new Cuerda(
                        generar_id(), cuerda->get_extremo_a(), sops_a, anc_b_a, lon_a
                    ));
                }
                // Bolita suelta desde torque_izq (penultimo soporte)
                double lon_s = Vector2D::distancia(puntos[seg_cortado-2], puntos[seg_cortado-1]);
                if (lon_s < 20.0) lon_s = 20.0;
                Vector2D pos_bs = puntos[seg_cortado-2] + Vector2D(0, lon_s * 0.3);
                Bola* bs = new Bola(generar_id(), pos_bs, 2.0, 0.3);
                bs->set_amortiguamiento(0.08);
                bs->set_restitucion(0.05);
                entidades_nuevas.push_back(bs);
                AnclajeCuerda anc_sa{ id_b_a, TipoAnclajeCuerda::SoporteFijo };
                AnclajeCuerda anc_sb{ bs->get_id(), TipoAnclajeCuerda::Cubeta };
                entidades_nuevas.push_back(new Cuerda(generar_id(), anc_sa, {}, anc_sb, lon_s));
            }

            // LADO B: cuerda [torque_der]->[GLOBO2] intacta + bolita suelta desde torque_der
            // El primer soporte del lado B (torque_izq) se descarta
            // El segundo soporte del lado B (torque_der) es el nuevo extremo_a
            if (num_sops_b >= 2) {
                int id_anc = sops[num_sops_a + 1]; // torque_der
                // Cuerda torque_der -> GLOBO2
                double lon_b = 0.0;
                for (int i = seg_cortado + 1; i < n - 1; ++i)
                    lon_b += Vector2D::distancia(puntos[i], puntos[i+1]);
                if (lon_b > MathUtils::EPSILON) {
                    std::vector<int> sops_b(sops.begin() + num_sops_a + 2, sops.end());
                    AnclajeCuerda anc_a_b{ id_anc, TipoAnclajeCuerda::SoporteFijo };
                    entidades_nuevas.push_back(new Cuerda(
                        generar_id(), anc_a_b, sops_b, cuerda->get_extremo_b(), lon_b
                    ));
                }
                // Bolita suelta desde torque_der
                double lon_s = Vector2D::distancia(puntos[seg_cortado+1], puntos[seg_cortado+2 < n ? seg_cortado+2 : n-1]);
                if (lon_s < 20.0) lon_s = 20.0;
                Vector2D pos_bs = puntos[seg_cortado+1] + Vector2D(0, lon_s * 0.3);
                Bola* bs2 = new Bola(generar_id(), pos_bs, 2.0, 0.3);
                bs2->set_amortiguamiento(0.08);
                bs2->set_restitucion(0.05);
                entidades_nuevas.push_back(bs2);
                AnclajeCuerda anc_sa2{ id_anc, TipoAnclajeCuerda::SoporteFijo };
                AnclajeCuerda anc_sb2{ bs2->get_id(), TipoAnclajeCuerda::Cubeta };
                entidades_nuevas.push_back(new Cuerda(generar_id(), anc_sa2, {}, anc_sb2, lon_s));
            }
        }

        tijera->set_ya_corto_cuerdas();
        tijera->resetear_activacion();
    }

    for (int id : ids_a_eliminar)
        remover_entidad(id);
    for (auto* nueva : entidades_nuevas)
        entidades.push_back(nueva);
}

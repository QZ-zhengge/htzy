import { createRouter, createWebHistory } from 'vue-router'
import { useUserStore } from '../stores/user'

const routes = [
  // ===== Public pages =====
  { path: '/', name: 'Home', component: () => import('../views/Home.vue') },
  { path: '/login', name: 'Login', component: () => import('../views/Login.vue') },
  { path: '/register', name: 'Register', component: () => import('../views/Register.vue') },
  { path: '/jobs', name: 'Jobs', component: () => import('../views/Jobs.vue') },
  { path: '/red-guide', name: 'RedGuide', component: () => import('../views/RedGuide.vue') },
  { path: '/red-guide/:id', name: 'RedGuideDetail', component: () => import('../views/RedGuideDetail.vue') },

  // ===== Student pages (require auth + verified) =====
  {
    path: '/dashboard', name: 'Dashboard',
    component: () => import('../views/Dashboard.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/resume', name: 'Resume',
    component: () => import('../views/Resume.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/interview/:id', name: 'Interview',
    component: () => import('../views/Interview.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/results/:id', name: 'Results',
    component: () => import('../views/Results.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/auto-delivery', name: 'AutoDelivery',
    component: () => import('../views/AutoDelivery.vue'),
    meta: { requiresAuth: true }
  },

  // ===== Admin pages =====
  {
    path: '/admin/login', name: 'AdminLogin',
    component: () => import('../views/admin/AdminLogin.vue')
  },
  {
    path: '/admin', name: 'AdminDashboard',
    component: () => import('../views/admin/AdminDashboard.vue'),
    meta: { requiresAdmin: true }
  },
  {
    path: '/admin/students', name: 'AdminStudents',
    component: () => import('../views/admin/AdminStudents.vue'),
    meta: { requiresAdmin: true }
  },
  {
    path: '/admin/scores', name: 'AdminScores',
    component: () => import('../views/admin/AdminScores.vue'),
    meta: { requiresAdmin: true }
  },
  {
    path: '/admin/super', name: 'AdminSuper',
    component: () => import('../views/admin/AdminSuper.vue'),
    meta: { requiresSuperAdmin: true }
  },
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

router.beforeEach((to, from, next) => {
  const userStore = useUserStore()

  // Admin route guard
  if (to.meta.requiresSuperAdmin) {
    if (!userStore.isLoggedIn || userStore.role !== 'super_admin') {
      next({ name: 'AdminLogin' }); return
    }
  } else if (to.meta.requiresAdmin) {
    if (!userStore.isLoggedIn || (userStore.role !== 'admin' && userStore.role !== 'super_admin')) {
      next({ name: 'AdminLogin' }); return
    }
  }

  // Student route guard
  if (to.meta.requiresAuth) {
    if (!userStore.isLoggedIn || !userStore.checkExpiry()) {
      userStore.logout()
      next({ name: 'Login', query: { redirect: to.fullPath } }); return
    }
  }
  next()
})

export default router

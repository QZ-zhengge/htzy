import { defineStore } from 'pinia'

const EXPIRY_HOURS = 5

export const useUserStore = defineStore('user', {
  state: () => {
    const token = sessionStorage.getItem('token')
    const loginTime = sessionStorage.getItem('loginTime')

    if (token && loginTime) {
      const elapsed = Date.now() - parseInt(loginTime, 10)
      const fiveHours = EXPIRY_HOURS * 60 * 60 * 1000
      if (elapsed > fiveHours) {
        sessionStorage.removeItem('token')
        sessionStorage.removeItem('user')
        sessionStorage.removeItem('loginTime')
        return { token: '', user: null }
      }
    } else if (!token) {
      return { token: '', user: null }
    }

    return {
      token: token || '',
      user: JSON.parse(sessionStorage.getItem('user') || 'null')
    }
  },

  getters: {
    isLoggedIn: (state) => !!state.token,
    username: (state) => state.user?.username || '',
    role: (state) => state.user?.role || 'student',
    isVerified: (state) => state.user?.is_verified === true,
    isAdmin: (state) => state.user?.role === 'admin' || state.user?.role === 'super_admin',
    isSuperAdmin: (state) => state.user?.role === 'super_admin',
  },

  actions: {
    setAuth(newToken, newUser) {
      this.token = newToken
      this.user = newUser
      sessionStorage.setItem('token', newToken)
      sessionStorage.setItem('user', JSON.stringify(newUser))
      sessionStorage.setItem('loginTime', Date.now().toString())
    },

    logout() {
      this.token = ''
      this.user = null
      sessionStorage.removeItem('token')
      sessionStorage.removeItem('user')
      sessionStorage.removeItem('loginTime')
    },

    checkExpiry() {
      const loginTime = sessionStorage.getItem('loginTime')
      if (!loginTime) return false
      const elapsed = Date.now() - parseInt(loginTime, 10)
      const fiveHours = EXPIRY_HOURS * 60 * 60 * 1000
      if (elapsed > fiveHours) {
        this.logout()
        return false
      }
      return true
    }
  }
})
